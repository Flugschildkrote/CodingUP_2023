#ifndef ANUBIS_UTIL_THREAD_POOL_HPP
#define ANUBIS_UTIL_THREAD_POOL_HPP

#include <cstdint>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include <vector>

namespace anubis
{
    class ThreadPool;

    template <typename T>
    class TaskResult {
        friend class ThreadPool;
    public:
        std::future<T> future;
    private:
        std::unique_ptr<std::promise<T>> promise;
    };

    class ThreadPool
    {
    public:
        /*
         * @brief Creates a thread pool with a fiex amount of threads
         * @param numThreads The number of threads in the pool. 0 mean automatic (ie. The maximum number of concurrent threads allowed by the machine)
         */
        ThreadPool(uint32_t numThreads = 0);
        ~ThreadPool(void);

        template <typename T_RESULT, typename T_TASK>
        TaskResult<T_RESULT> pushTask(T_TASK&& task);

        uint32_t getNumThreads(void) const noexcept;

    private:
        void terminateThreads(void);

        std::vector<std::thread> m_WorkerThreads;
        std::queue<std::function<void(void)>> m_TasksQueue;
        std::condition_variable m_TasksConditionVariable;
        std::mutex m_MainMutex;
        bool m_ExitRequested;
    };
}

#include "anubis/util/inl/ThreadPool.inl"

#endif // !ANUBIS_UTIL_THREAD_POOL_HPP