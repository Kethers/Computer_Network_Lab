#include "icmp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

#define ICMP_HEADER_LEN 8
#define IP_HEADER_LEN 20
#define ICMP_ERR_IP_DATA_LEN 8

// uint8_t last_src_ip[4] = {0};
// uint8_t my_pid = 0;
// uint8_t last_id = -1;

/**
 * @brief 处理一个收到的数据包
 *        你首先要检查buf长度是否小于icmp头部长度
 *        接着，查看该报文的ICMP类型是否为回显请求，
 *        如果是，则回送一个回显应答（ping应答），需要自行封装应答包。
 * 
 *        应答包封装如下：
 *        首先调用buf_init()函数初始化txbuf，然后封装报头和数据，
 *        数据部分可以拷贝来自接收到的回显请求报文中的数据。
 *        最后将封装好的ICMP报文发送到IP层。  
 * 
 * @param buf 要处理的数据包
 * @param src_ip 源ip地址
 */
void icmp_in(buf_t *buf, uint8_t *src_ip)
{
    // TODO
    icmp_hdr_t *icmp_in_hdr = (icmp_hdr_t *)buf->data;
    uint16_t icmp_checksum = icmp_in_hdr->checksum;
    icmp_in_hdr->checksum = 0;

    // 检包
    if (buf->len < ICMP_HEADER_LEN || icmp_checksum != checksum16((uint16_t *)buf->data, buf->len))
    {
        return;
    }

    if (icmp_in_hdr->type == ICMP_TYPE_ECHO_REQUEST && icmp_in_hdr->code == 0)
    {
        icmp_in_hdr->type = ICMP_TYPE_ECHO_REPLY;
        icmp_in_hdr->checksum = checksum16((uint16_t *)buf->data, buf->len);
        ip_out(buf, src_ip, NET_PROTOCOL_ICMP);
    }
}

/**
 * @brief 发送icmp不可达
 *        你需要首先调用buf_init初始化buf，长度为ICMP头部 + IP头部 + 原始IP数据报中的前8字节 
 *        填写ICMP报头首部，类型值为目的不可达
 *        填写校验和
 *        将封装好的ICMP数据报发送到IP层。
 * 
 * @param recv_buf 收到的ip数据包
 * @param src_ip 源ip地址
 * @param code icmp code，协议不可达或端口不可达
 */
void icmp_unreachable(buf_t *recv_buf, uint8_t *src_ip, icmp_code_t code)
{
    // TODO
    buf_init(&txbuf, ICMP_HEADER_LEN + IP_HEADER_LEN + ICMP_ERR_IP_DATA_LEN);
    icmp_hdr_t icmp_out_hdr = {
        .type = ICMP_TYPE_UNREACH,
        .code = code,
        .checksum = 0,
        .id = 0,
        .seq = 0};
    memcpy(txbuf.data, &icmp_out_hdr, ICMP_HEADER_LEN); //拷贝icmp头部(空校验和)
    //拷贝收到的ip数据包的头部和数据部分的8个字节
    memcpy(txbuf.data + ICMP_HEADER_LEN, recv_buf->data, IP_HEADER_LEN + ICMP_ERR_IP_DATA_LEN);
    icmp_out_hdr.checksum = checksum16((uint16_t *)txbuf.data, ICMP_HEADER_LEN + IP_HEADER_LEN + ICMP_ERR_IP_DATA_LEN);
    memcpy(txbuf.data, &icmp_out_hdr, ICMP_HEADER_LEN); //拷贝icmp头部(有校验和)
    ip_out(&txbuf, src_ip, NET_PROTOCOL_ICMP);
}