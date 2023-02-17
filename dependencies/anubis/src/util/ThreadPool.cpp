#include "anubis/util/ThreadPool.hpp"

namespace anubis 
{

//-----------------------------------------------
ThreadPool::ThreadPool(uint32_t numThreads)
    : m_ExitRequested(false)
{
    numThreads = numThreads ? numThreads : std::thread::hardware_concurrency();

    ThreadPool& pool = *this;
    auto workerFnc = [&pool] {

        std::unique_lock lk(pool.m_MainMutex);
        // here the mutex is aquired
        do
        {
            //  Wait for a job if the queue is empty. Will be woken up when pool request termination
            if (pool.m_TasksQueue.empty()) {
                pool.m_TasksConditionVariable.wait(lk, [&pool]() { return !pool.m_TasksQueue.empty() || pool.m_ExitRequested; });
            }

            // Exit if needed.
            if (pool.m_ExitRequested) {
                break;
            }

            // Pop the job and execute it
            std::function<void(void)> fnc = std::move(pool.m_TasksQueue.front());
            pool.m_TasksQueue.pop();

            lk.unlock();
            fnc();
            lk.lock();
        } while (true);

    };

    // Start worker threads
    m_WorkerThreads.resize(numThreads);
    for (std::thread& t : m_WorkerThreads) {
        t = std::thread(workerFnc);
    }
}

//-----------------------------------------------
ThreadPool::~ThreadPool(void) 
{
    terminateThreads();
}

//-----------------------------------------------
uint32_t ThreadPool::getNumThreads(void) const noexcept 
{
    return static_cast<uint32_t>(m_WorkerThreads.size());
}

//-----------------------------------------------
void ThreadPool::terminateThreads(void)
{
    {
        std::lock_guard lk(m_MainMutex);
        m_ExitRequested = true;
    }
    m_TasksConditionVariable.notify_all();

    for (std::thread& t : m_WorkerThreads) {
        t.join();
    }
}

}