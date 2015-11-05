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

    aggregator.Aggregate();

    return 0;
}

