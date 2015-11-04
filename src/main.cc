#include <iostream>

#include "link_aggregator.hh"
#include "nfqueue.hh"
#include "safe_queue.hh"
#include "piped_thread.hh"

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
    Buffer* buf;

    for(;;) {

        // Try to receive from client
        buf = aggregator.RecvPktFromClient();
        if(buf) {
//            std::cout << "Client --> Agg --> Links" << std::endl;
            // Got packet, forward to links
            aggregator.SendOnLinks(buf);
            delete buf;
        }

        // Try to receive from links
        buf = aggregator.RecvOnLinks();
        if(buf) {
//            std::cout << "Links --> Agg --> Client" << std::endl;
            // Got packet, forward to client
            aggregator.SendPktToClient(buf);
            delete buf;
        }
    }

    return 0;
}

