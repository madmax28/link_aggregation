#ifndef _TIMER_HH_
#define _TIMER_HH_

#include <cstdint>
#include <chrono>
#include <thread>

class Timer {

    std::thread m_t;

    template<typename F, typename... A>
    static void t(uint32_t msec, F fun, A... args) {
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
        fun(args...);
    }

    public:

    template<typename F, typename... A>
    Timer(uint32_t to_msec, F fun, A... args)
          : m_t(std::thread(t<F, A...>, to_msec, fun, args...)) {

        m_t.detach();
    }
};

#endif /* _TIMER_HH_ */
