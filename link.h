#ifndef _LINK_H_
#define _LINK_H_

#include <string>
#include <unistd.h>

#include <net/ethernet.h>

#include "common.h"

// Definition of our ethernet type/protocol
#define ETH_P_ALAGG 0x4242

/*
 * ALAGG Header definition
 */

struct __attribute__ ((__packed__)) AlaggHeader {
    struct ether_header m_eth_header;
    // TODO: SEQ no goes here
};

/*
 * ALAGG Packet definition
 */

struct __attribute__ ((__packed__)) AlaggPacket {
    AlaggHeader m_header;
    char m_payload[0];
};

/*
 * Class to manage the links to be aggregated.
 *
 * Utilizes a single socket for each link.
 * Link communication is managed within the data link layer.
 * Thus, sockets are bound to interfaces.
 */

class Link {

    /* Link properties */
    int m_socket;
    MacAddress m_peer_addr;
    MacAddress m_own_addr;

    public:

    Link( std::string const ifname,
       std::string const mac_addr_str );
    ~Link() {
        close(m_socket);
        m_socket = 0;
    }

    // Getters
    int const Socket() const { return m_socket; }
    MacAddress const PeerAddr() const { return m_peer_addr; }
    MacAddress const OwnAddr() const { return m_own_addr; }
};

#endif /* _LINK_H_ */
