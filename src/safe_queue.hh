#ifndef _SAFE_QUEUE_HH_
#define _SAFE_QUEUE_HH_

#include <queue>
#include <mutex>

template<typename T>
class SafeQueue {

    public:

    enum safe_queue_except {
        except_isempty = 0
    };

    std::queue<T> m_queue;
    std::mutex m_mutex;

    SafeQueue() {}

    void Push(const T& val) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(val);
    }

    void Pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_queue.empty())
            throw except_isempty;
        m_queue.pop();
    }

    T& PopFront() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_queue.empty())
            throw except_isempty;
        T &val = m_queue.front();
        m_queue.pop();
        return val;
    }

    T& Front() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_queue.empty())
            throw except_isempty;
        return m_queue.front();
    }

    const T& Front() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.front();
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};

#endif /* _SAFE_QUEUE_HH_ */
