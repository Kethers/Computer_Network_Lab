#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 处理一个收到的数据包
 *        你需要判断以太网数据帧的协议类型，注意大小端转换
 *        如果是ARP协议数据包，则去掉以太网包头，发送到arp层处理arp_in()
 *        如果是IP协议数据包，则去掉以太网包头，发送到IP层处理ip_in()
 *
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
    // TODO
    ether_hdr_t ether_header;
    memcpy(&ether_header, buf->data, 14);                    //让以太网头结构体等于buf的数据起始地址开始的14个字节
    uint16_t ether_protocol = swap16(ether_header.protocol); //协议16bit的大小端置换
    buf_remove_header(buf, 14);
    if (ether_protocol == NET_PROTOCOL_ARP)
    {
        arp_in(buf);
    }
    else if (ether_protocol == NET_PROTOCOL_IP)
    {
        ip_in(buf);
    }
}

/**
 * @brief 处理一个要发送的数据包
 *        你需添加以太网包头，填写目的MAC地址、源MAC地址、协议类型
 *        添加完成后将以太网数据帧发送到驱动层
 *
 * @param buf 要处理的数据包
 * @param mac 目标mac地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
    // TODO
    buf_add_header(buf, 14);
    ether_hdr_t ether_header;
    memcpy(ether_header.dest, mac, NET_MAC_LEN);            //mac地址拷给以太网头结构体的目的mac地址
    memcpy(ether_header.src, net_if_mac, NET_MAC_LEN);      //同理拷给源mac地址
    ether_header.protocol = swap16(protocol);               //协议16字节的大小端置换
    memcpy(buf->data, &ether_header, sizeof(ether_header)); //将填写好的以太网头结构体拷贝给buf的数据始址
    driver_send(buf);
}

/**
 * @brief 初始化以太网协议
 *
 * @return int 成功为0，失败为-1
 */
int ethernet_init()
{
    buf_init(&rxbuf, ETHERNET_MTU + sizeof(ether_hdr_t));
    return driver_open();
}

/**
 * @brief 一次以太网轮询
 *
 */
void ethernet_poll()
{
    if (driver_recv(&rxbuf) > 0)
        ethernet_in(&rxbuf);
}
