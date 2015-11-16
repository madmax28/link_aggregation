/** @file link.hh
 * Link class definition
 */

#ifndef _LINK_HH_
#define _LINK_HH_

#include <cstdint>
#include <string>
#include <unistd.h>
#include <limits.h>

#include <net/ethernet.h>

#include "common.hh"

/**
 * Defintion of the Alagg protocol's ethernet type.
 *
 * This is used by the kernel to sort out packets of this type. Our sockets will
 * be bound to this type.
 */
#define ETH_P_ALAGG 0x4242

/**
 * Definition of the maximum possible sequence number used by the Alagg
 * protocol.
 */
#define ALAGG_MAX_SEQ (USHRT_MAX)

/**
 * Sequence number type defninition
 */
typedef uint16_t alagg_seq_t;

/**
 * Time to live for packets that are received out-of-order in milliseconds.
 *
 * If a packet is received out-of-order, it will be deferred for at maximum this
 * amount of time to perform reordering. After the timeout occurs, the packet
 * will be delivered either way.
 */
#define ALAGG_REORDER_TTL 50

/**
 * ALAGG Header definition
 *
 * The header consists of the standard ethernet header plus a packet sequence
 * number.
 */
struct __attribute__ ((__packed__)) AlaggHeader {
    struct ether_header m_eth_header;
    alagg_seq_t m_seq;
};

/**
 * ALAGG Packet definition
 *
 * A packet consists of an AlaggHeader as well as the payload.
 */
struct __attribute__ ((__packed__)) AlaggPacket {
    AlaggHeader m_header;
    char m_payload[0];
};

/**
 * Link class
 *
 * Class to manage communication on aggregated links. The communication takes
 * place in the data link layer, and link sockets are bound to specific
 * interfaces.
 */
class Link {

    // Socket fd
    int         m_socket;
    // Peer's MAC address
    MacAddress  m_peer_addr;
    // Local MAC address
    MacAddress  m_own_addr;
    // Name of the interface, e.g. eth0
    std::string m_if_name;

    public:

    Link( std::string const ifname,
       std::string const mac_addr_str );

    /**
     * Link class deconstructor
     *
     * Closes the associated socket.
     */
    ~Link() {
        close(m_socket);
        m_socket = 0;
    }

    /**
     * Getter for the socket fd.
     * @returns The Socket file descriptor.
     */
    int const Socket() const { return m_socket; }

    /**
     * Getter for the peer's MAC address.
     * @returns MacAddress object.
     */
    MacAddress const PeerAddr() const { return m_peer_addr; }

    /**
     * Getter for the own MAC address.
     * @returns MacAddress object.
     */
    MacAddress const OwnAddr() const { return m_own_addr; }

    /**
     * Getter for the name of the interface bound to.
     * @returns String containing the interfaces name.
     */
    std::string const IfName() const { return m_if_name; }
};

#endif /* _LINK_HH_ */
