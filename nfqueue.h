#ifndef _NFQUEUE_H_
#define _NFQUEUE_H_

#include <sys/types.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

#define NFQUEUE_BUFSIZE 4096

class NfqHandler {

    struct nfq_handle   *m_nfq_handle;
    char                 m_nfq_buffer[NFQUEUE_BUFSIZE];
    struct nfq_q_handle *m_nfq_q_handle;
    struct nfnl_handle  *m_nfq_nl_handle;
    int                  m_nfq_nl_fd;

    public:

    NfqHandler( int (*p_cb_fun)(nfq_q_handle*, nfgenmsg*, nfq_data*, void*) );
    ~NfqHandler() { nfq_close(m_nfq_handle); }

    int HandlePacket();

    struct nfq_handle * NfqHandle() const {
        return m_nfq_handle;
    }
};

#endif /* _NFQUEUE_H_ */
