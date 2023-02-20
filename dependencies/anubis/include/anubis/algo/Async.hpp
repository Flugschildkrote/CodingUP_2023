#ifndef ANUBIS_ALGO_ASYNC_HPP
#define ANUBIS_ALGO_ASYNC_HPP

#include <anubis/util/ThreadPool.hpp>
#include <algorithm>

#ifdef min
#undef min
#endif // min


namespace anubis
{


template <typename T_ITERATION>
void parallelForN(anubis::ThreadPool& pool, T_ITERATION&& it, size_t end, size_t start = 0) {

    size_t numIteration = end - start;
    size_t numThreads = pool.getNumThreads();
    size_t numBlocks = numThreads * 4; // Multiply per 4 for (hopefully) better load balancing
    size_t blockSize = (numIteration + numBlocks - 1) / numBlocks;

    std::vector<anubis::TaskResult<void>> results(numBlocks);

    for (size_t blockId = 0; blockId < numBlocks; ++blockId) {

        size_t loopBegin = (blockId * blockSize) + start;
        size_t loopEnd = std::min(loopBegin + blockSize, end);

        results[blockId] = pool.pushTask<void>([it, loopBegin, loopEnd]() {
            for (size_t i = loopBegin; i < loopEnd; ++i) {
                it(i);
            }
            });
    }

    // Wait for end of execution
    for (auto& r : results) {
        r.future.wait();
    }
}


}

#endif // !ANUBIS_ALGO_ASYNC_HPP