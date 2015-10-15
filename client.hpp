#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <vector>
#include "common.hpp"
#include <netinet/ip.h>      // htons()

enum Protocol {
    UDP = 0,
    TCP
};

class Client {

    int m_socket;
    struct sockaddr_in m_sockaddr;
    int m_port;

    public:

    Client();
    ~Client();

    Buffer RecvPkt();
};

#endif /* _CLIENT_H_ */
