#ifndef _CLIENT_HH_
#define _CLIENT_HH_

#include <vector>

#include <netinet/ip.h>      // htons()

#include "common.hh"
#include "nfqueue.hh"

class Client {

    int m_socket;
    NfqHandler m_nfq_handler;

    public:

    Client();
    ~Client();

    Buffer RecvPkt();
    int SendPkt( Buffer const &buf ) const;
};

#endif /* _CLIENT_HH_ */
