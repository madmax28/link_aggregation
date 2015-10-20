#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <vector>
#include "common.h"
#include <netinet/ip.h>      // htons()

enum Protocol {
    UDP = 0,
    TCP
};

class Client {

    int m_socket;

    public:

    Client();
    ~Client();

    Buffer RecvPkt() const;
    int SendPkt( Buffer const &buf ) const;
};

#endif /* _CLIENT_H_ */
