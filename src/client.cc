#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "client.hh"

/**
 * Client class constructor.
 *
 * Opens an IP-layer socket for communication with the client application(s).
 * The socket is bound to the loopback interface and made non-blocking.
 */
Client::Client() {

    /*
     * Prepare socket for client connection
     */

    struct ifreq ifr;
    struct sockaddr_ll sll;

    // We want IP packets, including headers
    m_socket = socket( AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP) );
    assert_perror(errno);

    // Get interface index
    memset( &ifr, 0, sizeof( ifr) );
    strncpy( ifr.ifr_name, IFNAME_LOOPBACK, IFNAMSIZ - 1 );
    ioctl( m_socket, SIOCGIFINDEX, &ifr );
    assert_perror(errno);
    memset( &sll, 0, sizeof( sll) );
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_IP);

    // Bind the raw socket to the interface specified
    bind( m_socket, (struct sockaddr *)&sll, sizeof(sll) );
    assert_perror(errno);

    // Make socket non-blocking
    int fdflags = fcntl( m_socket, F_GETFL );
    assert_perror(errno);
    fcntl( m_socket, F_SETFL, fdflags | O_NONBLOCK );
    assert_perror(errno);
}

/**
 * Client class destructor.
 *
 * Closes the socket used for client communication
 */
Client::~Client() {
    close(m_socket);
}

/**
 * Try to receive a packet from the client.
 *
 * Get the raw packet from NfqHandler by calling GetPacket() and insert the data
 * into a new Buffer object.
 *
 * @return A pointer to the newly allocated Buffer object
 * @see NfqHandler
 */
Buffer * Client::RecvPkt() {

    Buffer *buf = nullptr;
    unsigned char *raw_buf;
    int pkt_len;

    pkt_len = GetPacket(&raw_buf);

    if( pkt_len > 0 ) {
        buf = new Buffer();
        buf->assign( raw_buf, raw_buf+pkt_len );
    }

    return buf;
}

/**
 * Send a packet to the client.
 *
 * @param buf Pointer to the Buffer object containing the message to be
 * sent.
 * @return The return value of the underlying send() call.
 */
int Client::SendPkt( Buffer const * buf ) const {

    int byte_sent;

    byte_sent = send(m_socket, buf->data(), buf->size(), 0);
    if(byte_sent == -1)
        perror("sent()");

    return byte_sent;
}
