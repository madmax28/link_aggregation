#include "../timer.hh"

#include <iostream>

static void cb(std::string m) {
    std::cout << "Received callback. m = " << m << "\n";
}

int main(int argc, const char *argv[]) {
    Timer t(5e3, cb, "Hello World!");

    for(;;){}

    return 0;
}
