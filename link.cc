#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>

#include "link.hpp"

Link::Link(int port) :
        m_port(port) {

    std::cout << "Port is " << port << std::endl;

    // Open the socket
//    m_socket = socket(
}
