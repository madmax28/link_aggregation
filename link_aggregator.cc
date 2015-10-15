#include <iostream>
#include <unistd.h> // sleep()
#include "link_aggregator.hpp"

LinkAggregator::LinkAggregator(int num_links) :
        /* Config defaults */
        m_clientport (4200) {

    std::cout << "Creating new aggregator with " << num_links << " links." << std::endl;

    // Init Links
    for( int i = 1; i <= num_links; i++ ) {
        m_links.push_back(Link(m_clientport+i));
    }
}

int main(int argc, const char *argv[]) {

    LinkAggregator aggregator;

    Buffer pktbuf;

    for(;;) {
        pktbuf.clear();

        std::cout << "Receiving.." << std::endl;
        pktbuf = aggregator.RecvPktFromClient();
        std::cout << "Received " << pktbuf.size() << " bytes" << std::endl;

        sleep(1);
    }

    return 0;
}

