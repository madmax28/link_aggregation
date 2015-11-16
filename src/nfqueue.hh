/** @file nfqueue.hh
 * NfqHandler class definition
 */

#ifndef _NFQUEUE_HH_
#define _NFQUEUE_HH_

#include <sys/types.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "common.hh"

/**
 * NfqHandler class.
 *
 * This class handles packet interception and reception via the netfilter and
 * the nfqueue libraries.
 * Please see the libraries' documentations for further information.
 */
class NfqHandler {

    /**
     * Arguments passed to the nfqueue callback function.
     *
     * The function will store the received packet in the buffer pointed to by
     * mp_packet and set m_packet_len according to the packets size. This way,
     * the packet is passed to upper layers of the application.
     */
    struct NfqCbArgs {
        unsigned char *mp_packet;
        int            m_packet_len;
    };

    // Nfqueue handles
    struct nfq_handle   *m_nfq_handle;
    char                 m_nfq_buffer[BUF_SIZE];
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

    /**
     * NfqHandler class desctructor
     *
     * Closes the associated Nfqueue handles
     */
    ~NfqHandler() {
        nfq_close(m_nfq_handle);
        nfq_destroy_queue(m_nfq_q_handle);
    }

    int GetPacket( unsigned char **packet_buffer );

    /**
     * Getter function for the file descriptor associated with the netfilter
     * queue.
     *
     * @returns The file descriptor of the queue.
     */
    int const NfqNlFd() const { return m_nfq_nl_fd; }
};

#endif /* _NFQUEUE_HH_ */
