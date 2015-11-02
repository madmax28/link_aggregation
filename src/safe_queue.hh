#ifndef _SAFE_QUEUE_HH_
#define _SAFE_QUEUE_HH_

#include <queue>
#include <mutex>

template<typename T>
class SafeQueue {

    std::queue<T> m_queue;
    std::mutex m_mutex;

    public:

    void Push(const T& val) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(val);
    }

    void Pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.pop();
    }

    T& Front() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.front();
    }

    const T& Front() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.front();
    }

    void Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};

#endif /* _SAFE_QUEUE_HH_ */
