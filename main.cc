#include <iostream>

#include "link_aggregator.h"
#include "nfqueue.h"

int main( int argc, const char *argv[] ) {

    std::string config_file;
    if( argc == 3 ) {
        if( std::string(argv[1]) != "-c" ) {
            std::cerr << "Usage: aggregator [-c <config_file>]" << std::endl;
            exit(1);
        }
        config_file = std::string(argv[2]);
    }

    LinkAggregator aggregator(config_file);
    Buffer pktbuf;

    std::string msg = "Hello World!\n";
    Buffer buf( msg.begin(), msg.end() );

    for(;;) {

        // Try to receive from client
        pktbuf.clear();
        pktbuf = aggregator.RecvPktFromClient();
        if( pktbuf.size() > 0 ) {
//            std::cout << "Client --> Agg --> Links" << std::endl;
            // Got packet, forward to links
            aggregator.SendOnLinks(pktbuf);
        }

        // Try to receive from links
        pktbuf.clear();
        pktbuf = aggregator.RecvOnLinks();
        if( pktbuf.size() > 0 ) {
//            std::cout << "Links --> Agg --> Client" << std::endl;
            // Got packet, forward to client
            aggregator.SendPktToClient(pktbuf);
        }
    }

    return 0;
}

