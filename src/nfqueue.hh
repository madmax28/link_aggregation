#ifndef _NFQUEUE_HH_
#define _NFQUEUE_HH_

#include <sys/types.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

#define NFQUEUE_BUFSIZE 4096

class NfqHandler {

    struct NfqCbArgs {
        unsigned char *mp_packet;
        int            m_packet_len;
    };

    struct nfq_handle   *m_nfq_handle;
    char                 m_nfq_buffer[NFQUEUE_BUFSIZE];
    struct nfq_q_handle *m_nfq_q_handle;
    struct nfnl_handle  *m_nfq_nl_handle;
    int                  m_nfq_nl_fd;
    NfqCbArgs            m_nfq_cb_args;

    static int NfqCallbackFun( struct nfq_q_handle *nfq,
            struct nfgenmsg *pkt,
            struct nfq_data *nfa,
            void *data );

    public:

    NfqHandler();
    ~NfqHandler() {
        nfq_close(m_nfq_handle);
        nfq_destroy_queue(m_nfq_q_handle);
    }

    int GetPacket( unsigned char **packet_buffer );

    int const NfqNlFd() const { return m_nfq_nl_fd; }
};

#endif /* _NFQUEUE_HH_ */
