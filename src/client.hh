#ifndef _CLIENT_HH_
#define _CLIENT_HH_

#include <vector>

#include <netinet/ip.h>      // htons()

#include "common.hh"
#include "nfqueue.hh"

class Client : NfqHandler {

    int m_socket;

    public:

    Client();
    ~Client();

    Buffer * RecvPkt();
    int SendPkt( Buffer const * buf ) const;

    int const RxFd() const { return NfqNlFd(); }
};

#endif /* _CLIENT_HH_ */
