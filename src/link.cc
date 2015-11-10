#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/if_packet.h> // AF_PACKET
#include <netinet/ip.h>      // htons()
#include <arpa/inet.h>       // inet_ntoa()

#include <net/if.h>          // Binding a socket to an interface
#include <sys/ioctl.h>

#include <fcntl.h>           // fcntl()

#include "link.hh"

Link::Link( std::string const ifname,
        std::string const mac_addr_str )
        : m_peer_addr(mac_addr_str)
        , m_if_name(ifname) {

    /*
     * Link sockets are of type
     *   socket( AF_PACKET, SOCK_RAW, ETH_P_ALAGG )
     * These sockets include the ethernet header, which we
     * to fill out manually before sending.
     * The ethernet header consists of:
     *   - source MAC address
     *   - destination MAC address
     *   - ethernet type
     */
    m_socket = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALAGG) );
    assert_perror(errno);

    /*
     * Bind the socket to the provided interface
     */

    struct ifreq ifr;
    struct sockaddr_ll sll;
    memset( &sll, 0, sizeof( sll) );
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALAGG);

    // Get interface index
    memset( &ifr, 0, sizeof( ifr) );
    strncpy( ifr.ifr_name, ifname.c_str(), IFNAMSIZ - 1 );
    ioctl( m_socket, SIOCGIFINDEX, &ifr );
    assert_perror(errno);
    sll.sll_ifindex = ifr.ifr_ifindex;

    // Get hardware address
    ioctl( m_socket, SIOCGIFHWADDR, &ifr );
    assert_perror(errno);
    char tmp[MAC_ADDR_STRLEN+1];
    snprintf( (char *) tmp, MAC_ADDR_STRLEN+1,
            "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
            ifr.ifr_hwaddr.sa_data[0],
            ifr.ifr_hwaddr.sa_data[1],
            ifr.ifr_hwaddr.sa_data[2],
            ifr.ifr_hwaddr.sa_data[3],
            ifr.ifr_hwaddr.sa_data[4],
            ifr.ifr_hwaddr.sa_data[5] );
    m_own_addr.SetAddr( std::string(tmp) );

    // Bind the raw socket to the interface specified
    bind( m_socket, (struct sockaddr *)&sll, sizeof(sll) );
    assert_perror(errno);

    // Make socket non-blocking
    int fdflags = fcntl( m_socket, F_GETFL );
    assert_perror(errno);
    fcntl( m_socket, F_SETFL, fdflags | O_NONBLOCK );
    assert_perror(errno);
}
