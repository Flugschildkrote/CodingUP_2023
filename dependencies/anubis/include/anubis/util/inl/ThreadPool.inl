#ifndef ANUBIS_UTIL_THREAD_POOL_HPP
#include "anubis/util/ThreadPool.hpp"
#endif // !ANUBIS_UTIL_THREAD_POOL_HPP

namespace anubis
{

template <typename T_RESULT, typename T_TASK>
TaskResult<T_RESULT> ThreadPool::pushTask(T_TASK&& task) {
    TaskResult<T_RESULT> res;
    res.promise = std::make_unique<std::promise<T_RESULT>>();
    std::promise<T_RESULT>* pPromise = res.promise.get();
    res.future = res.promise->get_future();

    m_MainMutex.lock();
    bool queueWasEmpty = m_TasksQueue.empty();
    m_TasksQueue.push([pPromise, task]() {
        if constexpr (std::is_void_v<T_RESULT>) {
            task();
            pPromise->set_value();
        }
        else {
            pPromise->set_value(task());
        }
        });
    m_MainMutex.unlock();

    m_TasksConditionVariable.notify_one();

    return res;
}

}