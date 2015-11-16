/** @file link_manager.hh
 * LinkManager class definition
 */

#ifndef _LINK_MANAGER_HH_
#define _LINK_MANAGER_HH_

#include "piped_thread.hh"
#include "safe_queue.hh"
#include "link.hh"
#include "packet_pool.hh"
#include "common.hh"

#include <vector>
#include <string>
#include <string.h>
#include <poll.h>

/**
 * Definition of poll events used to poll on link status.
 *
 * These include any data reception.
 */
#define LINK_POLL_EVENTS (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI)

/**
 * LinkManager class
 *
 * The LinkManager class handles the aggregated links, which are stored as
 * instances of Link. The implementation is multi-threaded. Once the class is
 * constructed, a new PipedThread is spawned that continuously calls
 * LinkManager::recv_on_links().
 *
 * Once a packet is received, it is pushed to the PacketPool, via
 * PacketPool::Add(). The PacketPool will push it to a SafeQueue, or defer the
 * packet if it was received out-of-order and push it at a later time.
 * After a packet was pushed to the SafeQueue, the pipe openend by PipedThread
 * will be notified.
 *
 * LinkManager inherits from three classes
 *   - SafeQueue, thread safe queue to which packets are pushed after reception
 *   - PipedThread, performing link reception and pipe notification
 *   - PacketPool, temporary pool of out-of-order packets
 *
 * @see Link
 * @see SafeQueue
 * @see PipedThread
 * @see PacketPool
 */
class LinkManager
        : public SafeQueue<Buffer *>
        , public PipedThread
        , public PacketPool {

    // Vector of Links to be aggregated
    std::vector<Link *>  m_links;
    // Used for asynchronous I/O on Client and Link reception
    struct pollfd       *m_link_pfds;
    nfds_t               m_link_nfds;

    // Transmission sequence number
    alagg_seq_t          m_tx_seq;

    static void recv_on_links(LinkManager *t);

    /**
     * Tx sequence number incrementation.
     *
     * @returns The next tx sequence number to be used
     */
    alagg_seq_t NextTxSeq() { return (m_tx_seq++ % USHRT_MAX); }

    /**
     * Pushes a packet to SafeQueue, and notifies the pipe.
     *
     * @param b Packet buffer to be pushed.
     * @see SafeQueue
     * @see PipedThread
     */
    void PopPacketFromPool(Buffer * b) {
        Push(b);
        NotifyPipe();
    }

    public:

    LinkManager(std::vector<std::string> peer_addresses,
                std::vector<std::string> if_names);
    ~LinkManager();

    // Link communication
    int Send(Buffer const * buf);
    Buffer const * Recv();

    /**
     * Getter for the links.
     *
     * @returns A vector of Link objects, containing the aggregated links.
     * @see Link
     */
    std::vector<Link *> const Links() const { return m_links; }
};

#endif /* _LINK_MANAGER_HH_ */
