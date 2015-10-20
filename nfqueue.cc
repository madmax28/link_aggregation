#include <iostream>
#include <stdlib.h>

#include "nfqueue.h"

#define NFQ_QUEUE_NUM 0

NfqHandler::NfqHandler( int (*p_cb_fun)(nfq_q_handle*, nfgenmsg*, nfq_data*, void*) )
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
            p_cb_fun,
            NULL );
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
}

int NfqHandler::HandlePacket() {

    int ret = recv( m_nfq_nl_fd, m_nfq_buffer, sizeof(m_nfq_buffer), 0 );
    if( ret > 0 ) {
        std::cout << "Received byte: " << ret << std::endl;
        nfq_handle_packet( m_nfq_handle,
                m_nfq_buffer,
                ret );
    }   
    return ret;
}
