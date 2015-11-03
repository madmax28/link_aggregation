#include "../piped_thread.hh"
#include "../safe_queue.hh"

#include <sstream>
#include <iostream>

static unsigned long long n = 0;

static void push(SafeQueue<int> *q) {

    std::stringstream m;

    q->Push(0);
    m << "Pushed. n=" << ++n << "\n";
    std::cout << m.str();
}

static void pop(SafeQueue<int> *q) {

    std::stringstream m;

    try {
        q->PopFront();
        m << "Popped. n=" << (n == 0 ? 0 : --n) << std::endl;
        std::cout << m.str();
    }
    catch(...) {}
}

int main() {
    SafeQueue<int> q;

    PipedThread t3; // Empty

    // Set t3 now
    t3.SetThread(pop, &q, PipedThread::exec_repeat);

    PipedThread t1(push, &q, PipedThread::exec_repeat);
    PipedThread t2(pop, &q, PipedThread::exec_repeat);

    t1.Join();
    t2.Join();
    t3.Join();

    return 0;
}
