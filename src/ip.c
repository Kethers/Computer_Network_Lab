#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include "driver.h"
#include "ethernet.h"
#include <string.h>

#define IP_HEADER_LEN 20
#define IP_DATA_MAX_LEN (ETHERNET_MTU - IP_HEADER_LEN)

int id_out = 0;
extern int arp_buf_sendout_flag;

/**
 * @brief 处理一个收到的数据包
 *        你首先需要做报头检查，检查项包括：版本号、总长度、首部长度等。
 * 
 *        接着，计算头部校验和，注意：需要先把头部校验和字段缓存起来，再将校验和字段清零，
 *        调用checksum16()函数计算头部检验和，比较计算的结果与之前缓存的校验和是否一致，
 *        如果不一致，则不处理该数据报。
 * 
 *        检查收到的数据包的目的IP地址是否为本机的IP地址，只处理目的IP为本机的数据报。
 * 
 *        检查IP报头的协议字段：
 *        如果是ICMP协议，则去掉IP头部，发送给ICMP协议层处理
 *        如果是UDP协议，则去掉IP头部，发送给UDP协议层处理
 *        如果是本实验中不支持的其他协议，则需要调用icmp_unreachable()函数回送一个ICMP协议不可达的报文。
 *          
 * @param buf 要处理的包
 */
void ip_in(buf_t *buf)
{
    // TODO
    ip_hdr_t *ip = (ip_hdr_t *)buf->data;

    uint16_t total_len = swap16(ip->total_len);
    uint16_t id = swap16(ip->id);
    uint16_t flags_fragment = swap16(ip->flags_fragment);

    // 检包
    if (ip->version != IP_VERSION_4               //这堆斜杠是为了防止插件自动格式化的时候把它们合成一行
        || ip->hdr_len < 20 / IP_HDR_LEN_PER_BYTE //
        // 服务类型中的4位TOS字段最多只能有1个置1，且最后1位保留字段必须为0
        || (((ip->tos & 0b10000) >> 5) + ((ip->tos & 0b1000) >> 4) + ((ip->tos & 0b100) >> 3) + ((ip->tos & 0b10) >> 2) > 1) || ((ip->tos & 0b1) != 0) //
        || total_len < 20                                                                                                                              //
        // 如果MF=1，则该数据包总长度应该是8的倍数
        || (flags_fragment & IP_MORE_FRAGMENT) && ((total_len - IP_HEADER_LEN) % 8 != 0) //
        || ip->ttl <= 0                                                                  //
        // || (ip->protocol != NET_PROTOCOL_ICMP && ip->protocol != NET_PROTOCOL_TCP && ip->protocol != NET_PROTOCOL_UDP)   //
        || memcmp(ip->dest_ip, &net_if_ip, NET_IP_LEN) //目的IP是否为本机IP,返回0则是，否则不是
        || checksum16((uint16_t *)ip, IP_HEADER_LEN)   //头部校验和为0则没问题，否则检验不通过
    )
    {
        return;
    }

    uint8_t src_ip[4];
    memcpy(&src_ip, ip->src_ip, NET_IP_LEN);
    if (ip->protocol == NET_PROTOCOL_ICMP || ip->protocol == NET_PROTOCOL_UDP)
    {
        buf_remove_header(buf, IP_HEADER_LEN);
        if (ip->protocol == NET_PROTOCOL_ICMP)
        {
            icmp_in(buf, src_ip);
        }
        else if (ip->protocol == NET_PROTOCOL_UDP)
        {
            udp_in(buf, src_ip);
        }
    }
    else // 不可识别的协议类型
    {
        icmp_unreachable(buf, src_ip, ICMP_CODE_PROTOCOL_UNREACH);
    }
}

/**
 * @brief 处理一个要发送的分片
 *        你需要调用buf_add_header增加IP数据报头部缓存空间。
 *        填写IP数据报头部字段。
 *        将checksum字段填0，再调用checksum16()函数计算校验和，并将计算后的结果填写到checksum字段中。
 *        将封装后的IP数据报发送到arp层。
 * 
 * @param buf 要发送的分片
 * @param ip 目标ip地址
 * @param protocol 上层协议
 * @param id 数据包id
 * @param offset 分片offset，必须被8整除
 * @param mf 分片mf标志，是否有下一个分片
 */
void ip_fragment_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol, int id, uint16_t offset, int mf)
{
    // TODO
    buf_add_header(buf, IP_HEADER_LEN);
    ip_hdr_t ip_header = {
        .version = IP_VERSION_4,
        .hdr_len = 5, //IP_HEADER_LEN / IP_HDR_LEN_PER_BYTE
        .tos = 0,
        .total_len = swap16(buf->len),
        .id = swap16(id),
        .flags_fragment = swap16(offset + (mf << 13)),
        .ttl = IP_DEFALUT_TTL,
        .protocol = protocol,
        .hdr_checksum = 0};

    memcpy(ip_header.src_ip, net_if_ip, NET_IP_LEN);
    memcpy(ip_header.dest_ip, ip, NET_IP_LEN);
    ip_header.hdr_checksum = checksum16((uint16_t *)&ip_header, IP_HEADER_LEN);
    memcpy(buf->data, &ip_header, IP_HEADER_LEN);
    arp_out(buf, ip, NET_PROTOCOL_IP);
}

/**
 * @brief 处理一个要发送的数据包
 *        你首先需要检查需要发送的IP数据报是否大于以太网帧的最大包长（1500字节 - ip包头长度）。
 *        
 *        如果超过，则需要分片发送。 
 *        分片步骤：
 *        （1）调用buf_init()函数初始化buf，长度为以太网帧的最大包长（1500字节 - ip包头头长度）
 *        （2）将数据报截断，每个截断后的包长度 = 以太网帧的最大包长，调用ip_fragment_out()函数发送出去
 *        （3）如果截断后最后的一个分片小于或等于以太网帧的最大包长，
 *             调用buf_init()函数初始化buf，长度为该分片大小，再调用ip_fragment_out()函数发送出去
 *             注意：最后一个分片的MF = 0
 *    
 *        如果没有超过以太网帧的最大包长，则直接调用调用ip_fragment_out()函数发送出去。
 * 
 * @param buf 要处理的包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
    // TODO
    int buf_size = buf->len;
    if (buf_size > IP_DATA_MAX_LEN)
    {
        // buf_size -= IP_HEADER_LEN;
        int offset = 0;

        while (buf_size > 0)
        {
            buf_t ip_buf;
            if (buf_size > IP_DATA_MAX_LEN) //待发送的数据长度大于IP数据包数据部分的最大长度
            {
                buf_init(&ip_buf, IP_DATA_MAX_LEN);
                memcpy(ip_buf.data, buf->data, IP_DATA_MAX_LEN);
                ip_fragment_out(&ip_buf, ip, protocol, id_out, offset, 1);
                buf_remove_header(buf, IP_DATA_MAX_LEN); //删除原始数据包中已经发送的部分
                offset += ((IP_DATA_MAX_LEN) >> 3);
                buf_size -= IP_DATA_MAX_LEN; //更改待发送数据部分的大小

                //以下代码用于解决UDP实验的思考题3：即arp表中无目标ip时，因为此程序为单线程，
                //导致存储在arp_buf中的ip的第一个待发送的分片会被后续分片覆盖的问题
                // uint8_t *mac_hit_ARP = arp_lookup(ip);
                // if (mac_hit_ARP == NULL)
                // {
                //     arp_buf_sendout_flag = 0;
                //     buf_init(&rxbuf, ETHERNET_MTU + sizeof(ether_hdr_t));
                //     while (driver_recv(&rxbuf) > 0)
                //     {
                //         ethernet_in(&rxbuf);
                //         if (arp_buf_sendout_flag == 1)
                //         {
                //             break;
                //         }
                //     }
                // }
            }
            else
            {
                buf_init(&ip_buf, buf_size);
                memcpy(ip_buf.data, buf->data, buf_size);
                ip_fragment_out(&ip_buf, ip, protocol, id_out, offset, 0);
                buf_size = 0;
            }
        }
    }
    else
    {
        ip_fragment_out(buf, ip, protocol, id_out, 0, 0);
    }
    id_out++;
}
