#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>

#include "nfqueue.hh"

/**
 * The id of the netfilter queue that will be used.
 */
#define NFQ_QUEUE_NUM 0

/**
 * The Nfqueue callback function.
 *
 * This function is called when a packet is handle via nfq_handle_packet(). We
 * simply receive packet, store it in the NfqCbArgs structure, and tell the
 * kernel to drop it.
 *
 * @param nfq Nfqueue handle
 * @param pkt Nfqueue packet data
 * @param nfa Nfqueue packet meta data
 * @param data User-specified pointer to arguments. This is a pointer to
 * NfqCbARgs.
 * @returns The return value of the underlying nfq_set_verdict() call.
 */
int NfqHandler::NfqCallbackFun( struct nfq_q_handle *nfq,
        struct nfgenmsg *pkt,
        struct nfq_data *nfa,
        void *data ) {

    NfqCbArgs *args = (NfqCbArgs *) data;

    // Get stuff from nfqueue
    struct nfqnl_msg_packet_hdr *nf_header;
    nf_header = nfq_get_msg_packet_hdr(nfa);
    uint32_t id = ntohl(nf_header->packet_id);

    // Store the packet in NfqCbArgs
    int ret = nfq_get_payload( nfa, &(args->mp_packet) );
    if( ret == -1 ) {
        std::cerr << "ERROR: could not receive payload" << std::endl;
    }
    args->m_packet_len = ret;

    // We tell the kernel to drop the packet here
    return nfq_set_verdict( nfq, id, NF_DROP, 0, NULL);
}

/**
 * NfqHandler class constructor
 *
 * Sets up the Nfqueue library.
 */
NfqHandler::NfqHandler()
        : m_nfq_handle(nfq_open()) {

    if(!m_nfq_handle) {
        std::cerr << "ERROR: could not open nfqueue handler\n";
        exit(1);
    }

    // Unbind from processing any IP packets
    if( nfq_unbind_pf( m_nfq_handle, AF_INET ) < 0 ) {
        std::cerr << "ERROR: could not unbind handle" << std::endl;
        exit(1);
    }

    // Bind to IP
    if( nfq_bind_pf( m_nfq_handle, AF_INET ) < 0 ) {
        std::cerr << "ERROR: could not bind handle" << std::endl;
        exit(1);
    }

    // Register nfq callback
    m_nfq_q_handle = nfq_create_queue( m_nfq_handle,
            NFQ_QUEUE_NUM,
            &NfqCallbackFun,
            (void *) &m_nfq_cb_args );
    if(!m_nfq_q_handle) {
        std::cerr << "ERROR: could not create nfqueue queue" << std::endl;
        exit(1);
    }

    // Set packet copy mode
    if( nfq_set_mode(m_nfq_q_handle, NFQNL_COPY_PACKET, 0xffff) == -1 ) {
        std::cerr << "ERROR: could not set packet copy mode" << std::endl;
        exit(1);
    }

    // Get netlink handle
    m_nfq_nl_handle = nfq_nfnlh(m_nfq_handle);
    if(!m_nfq_nl_handle) {
        std::cerr << "ERROR: could not get nfnl handle" << std::endl;
    }

    // Get netlink fd
    m_nfq_nl_fd = nfnl_fd(m_nfq_nl_handle);
    if(!m_nfq_nl_fd) {
        std::cerr << "ERROR: could not get nfnl fd" << std::endl;
    }

    int fdflags = fcntl( m_nfq_nl_fd, F_GETFL );
    if( fdflags < 0 ) {
        perror("fcntl()");
        exit(1);
    }
    if ( fcntl( m_nfq_nl_fd, F_SETFL, fdflags | O_NONBLOCK ) < 0 ) {
        perror("fcntl()");
        exit(1);
    }
}

/**
 * Receive a packet from the netfilter queue.
 *
 * @param packet Pointer to the buffer to be filled with the packet data.
 * @returns The size of the received packet.
 */
int NfqHandler::GetPacket( unsigned char **packet ) {

    int ret = recv( m_nfq_nl_fd, m_nfq_buffer, sizeof(m_nfq_buffer), 0 );
    if( ret > 0 ) {
        nfq_handle_packet( m_nfq_handle,
                m_nfq_buffer,
                ret );

        *packet = m_nfq_cb_args.mp_packet;
        return m_nfq_cb_args.m_packet_len;
    }

    return 0;
}
