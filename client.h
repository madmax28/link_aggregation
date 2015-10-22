#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <vector>

#include <netinet/ip.h>      // htons()

#include "common.h"
#include "nfqueue.h"

class Client {

    int m_socket;
    NfqHandler m_nfq_handler;

    public:

    Client();
    ~Client();

    Buffer RecvPkt();
    int SendPkt( Buffer const &buf ) const;
};

#endif /* _CLIENT_H_ */
