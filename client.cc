#include <iostream>
#include <stdio.h>           // perror()
#include <string.h>          // bzero(), memcpy()
#include <unistd.h>          // close()
#include <stdlib.h>          // exit()

#include <linux/if_packet.h> // AF_PACKET
#include <net/ethernet.h>    // ETH_P_IP
#include <netinet/ip.h>      // htons()
#include <arpa/inet.h>       // inet_ntoa()

#include <net/if.h>          // Binding a socket to an interface
#include <sys/ioctl.h>

#include <fcntl.h>           // fcntl()

#include <linux/netfilter_ipv4.h>

#include "client.hpp"
#include "common.hpp"

#define IFNAME_LOOPBACK "lo"

#define IP_HEADER_OFFSET 0
#define IP_HEADER_DST_ADDR_OFFSET 16

Client::Client() {
    /*
     * For the client connection, we want
     *
     *   socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))
     *
     * This will anything using IP
     */

    /* Prepare socket for client connection */

//    struct ifreq ifr;
//    struct sockaddr_ll sll;

    // We want IP packets, including headers
//    m_socket = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
//    if(m_socket == -1) {
//        perror("socket()");
//        exit(1);
//    }

    // Get interface index
//    memset(&ifr, 0, sizeof(ifr));
//    strncpy(ifr.ifr_name, "lo", IFNAMSIZ - 1);
//    if (ioctl(m_socket, SIOCGIFINDEX, &ifr) < 0){
//        perror("ioctl()");
//        exit(1);
//    }
//    memset(&sll, 0, sizeof(sll));
//    sll.sll_family = AF_PACKET;
//    sll.sll_ifindex = ifr.ifr_ifindex;
//    sll.sll_protocol = htons(ETH_P_IP);

    // Bind the raw socket to the interface specified
//    if (bind(m_socket, (struct sockaddr *)&sll, sizeof(sll)) < 0){
//        perror("bind()");
//        exit(1);
//    }

    // Make socket non-blocking
//    int fdflags = fcntl(m_socket, F_GETFL);
//    if(fdflags < 0) {
//        perror("fcntl()");
//        exit(1);
//    }
//    if (fcntl(m_socket, F_SETFL, fdflags | O_NONBLOCK) < 0) {
//        perror("fcntl()");
//        exit(1);
//    }

    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(m_socket == -1) {
        perror("socket()");
        exit(1);
    }

    bzero(&m_sockaddr, sizeof(m_sockaddr));
    m_sockaddr.sin_family = AF_INET;
    m_sockaddr.sin_port = htons(4200);
    m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_socket, (struct sockaddr *)&m_sockaddr, sizeof(m_sockaddr)) < 0){
        perror("bind()");
        exit(1);
    }

}

Client::~Client() {
    close(m_socket);
}

Buffer Client::RecvPkt() {
    Buffer buf;
    unsigned char raw_buf[BUF_SIZE];
    ssize_t bytes_recvd;

    bytes_recvd = recv(m_socket, (void *) raw_buf, BUF_SIZE, 0);
    if (bytes_recvd > 0)
        buf.insert(buf.end(), raw_buf, raw_buf+bytes_recvd);

    socklen_t addr_len = sizeof(m_sockaddr);
    int ret = getsockopt(m_socket,
            IPPROTO_IP,
            SO_ORIGINAL_DST,
            (struct sockaddr *) &m_sockaddr,
            &addr_len);
    if(ret == -1)
        perror("getsockopt()");

    std::cout << "Original destination: " << inet_ntoa(m_sockaddr.sin_addr) << std::endl;

    /*
     * Note: Iptables replaces the original destination address with
     * 127.0.0.1. Therefore, we will reinject the original destination address
     * into the ip header
     */

//    if(buf.size() > 0) {
//        unsigned char *p_dst_ip = buf.data() + IP_HEADER_OFFSET + IP_HEADER_DST_ADDR_OFFSET;
//        struct in_addr *dst_ip;
//        dst_ip = (struct in_addr *) p_dst_ip;

//        std::cout << "Dest address: " << inet_ntoa(*dst_ip) << std::endl;
//    }


    return buf;
}
