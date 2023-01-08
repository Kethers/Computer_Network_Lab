#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#define BUFFER_MAX 2048
#define IP_MAC 0x0800
#define ARP_MAC 0x0806
#define RARP_MAC 0x0835

int main(int argc, char *argv[])
{
    int fd;
    int proto;
    int str_len;
    char buffer[BUFFER_MAX];
    char *ethernet_head;
    char *ip_head;
    int mac_type;
    unsigned char *p;

    if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        printf("error create raw socket\n");
        return -1;
    }

    while (1)
    {
        str_len = recvfrom(fd, buffer, 2048, 0, NULL, NULL);

        if (str_len < 42)
        {
            printf("error when recv msg \n");
            return -1;
        }

        // MAC帧头
        ethernet_head = buffer;
        p = ethernet_head;
        printf("Src MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
               p[6], p[7], p[8], p[9], p[10], p[11]);
        printf("Dst MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
               p[0], p[1], p[2], p[3], p[4], p[5]);

        p = ethernet_head + 12;        //指向MAC帧的类型开头
        mac_type = (p[0] << 8) + p[1]; //MAC帧的数据包类型

        switch (mac_type)
        {
        case IP_MAC: //ip数据包
        {
            // ip数据包的头
            ip_head = ethernet_head + 14; // +14个字节的偏移,得到ip数据的起点
            p = ip_head + 12;
            printf("Src IP: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
            printf("Dst IP: %d.%d.%d.%d\n", p[4], p[5], p[6], p[7]);
            proto = (ip_head + 9)[0]; // +9个字节的偏移后到达协议部分

            printf("Protocol:");
            switch (proto)
            {
            case IPPROTO_ICMP:
            {
                printf("icmp\n");

                p = ip_head + 20; //移动到ICMP头部
                printf("Type: %x\n", p[0]);

                p += 1;
                printf("Code: %x\n", p[0]);

                p += 2; //2个字节的数字(用于表示校验和)
                printf("Check sum: %x\n", (p[0] << 8) + p[1]);

                p += 2;
                printf("Flag: %x\n", (p[0] << 8) + p[1]);

                p += 2;
                printf("Sequence: %x\n", (p[0] << 8) + p[1]);

                break;
            }

            case IPPROTO_TCP:
            {
                printf("tcp\n");
                break;
            }

            case IPPROTO_UDP:
            {
                printf("udp\n");
                break;
            }

            default:
                break;
            }
            break;
        }

        case ARP_MAC: //ARP请求/应答
        {
            printf("Protocol: ARP\n");
            p = ethernet_head + 14;
            printf("Hardware type: %x\n", (p[0] << 8) + p[1]);

            p += 2;
            printf("Protocol type: %x\n", (p[0] << 8) + p[1]);

            p += 2;
            printf("Hardware size: %x\n", p[0]);

            p += 1;
            printf("Protocol size: %x\n", p[0]);

            p += 1;
            printf("Opcode request %x\n", (p[0] << 8) + p[1]);

            p += 2;
            printf("Sender MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
                   p[0], p[1], p[2], p[3], p[4], p[5]);

            p += 6;
            printf("Sender IP address: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);

            p += 4;
            printf("Target MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
                   p[0], p[1], p[2], p[3], p[4], p[5]);

            p += 6;
            printf("Target IP address: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
        }

        default:
            break;
        }
        printf("-----------------------------------\n");
    }

    return -1;
}