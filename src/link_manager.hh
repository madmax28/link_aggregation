#ifndef _LINK_MANAGER_HH_
#define _LINK_MANAGER_HH_

#include "piped_thread.hh"
#include "safe_queue.hh"
#include "link.hh"
#include "common.hh"

#include <vector>
#include <string>
#include <string.h>
#include <poll.h>

#define LINK_POLL_EVENTS (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI)

class LinkManager
        : public SafeQueue<Buffer *>
        , public PipedThread {

    // Stores reordered packets ready to be delivered to the client
    std::vector<Link *>  m_links;
    struct pollfd       *m_link_pfds;
    nfds_t               m_link_nfds;

    unsigned short       m_tx_seq;
    unsigned short       m_rx_seq;

    static bool recv_on_links(LinkManager *t);
    bool IsInSequence( unsigned short const seq ) const;
    unsigned short NextTxSeq() { return (m_tx_seq++ % USHRT_MAX); }

    public:

    LinkManager(std::vector<std::string> peer_addresses,
                std::vector<std::string> if_names);
    ~LinkManager();

    int Send(Buffer const * buf);
    Buffer const * Recv();

    std::vector<Link *> const Links() const { return m_links; }
};

#endif /* _LINK_MANAGER_HH_ */
