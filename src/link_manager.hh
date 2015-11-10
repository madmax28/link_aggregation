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

#define LINK_POLL_EVENTS (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI)

class LinkManager
        : public SafeQueue<Buffer *>
        , public PipedThread
        , public PacketPool {

    std::vector<Link *>  m_links;
    struct pollfd       *m_link_pfds;
    nfds_t               m_link_nfds;

    alagg_seq_t          m_tx_seq;

    static void recv_on_links(LinkManager *t);
    alagg_seq_t NextTxSeq() { return (m_tx_seq++ % USHRT_MAX); }

    void PopPacketFromPool(Buffer * b) {
        Push(b);
        NotifyPipe();
    }

    public:

    LinkManager(std::vector<std::string> peer_addresses,
                std::vector<std::string> if_names);
    ~LinkManager();

    int Send(Buffer const * buf);
    Buffer const * Recv();

    std::vector<Link *> const Links() const { return m_links; }
};

#endif /* _LINK_MANAGER_HH_ */
