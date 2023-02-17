#ifndef ANUBIS_TIMER_HPP
#define ANUBIS_TIMER_HPP

#include <chrono>

namespace anubis
{

    class Timer
    {
    public:
        void start(void) {
            m_StartPoint = std::chrono::system_clock::now();
        }
        void stop(void) {
            m_EndPoint = std::chrono::system_clock::now();
        }

        float getSeconds(void) const noexcept {
            return std::chrono::duration<float>(m_EndPoint - m_StartPoint).count();
        }
        float getMillis(void) const noexcept {
            return std::chrono::duration<float, std::milli>(m_EndPoint - m_StartPoint).count();
        }
    private:

        std::chrono::time_point<std::chrono::system_clock> m_StartPoint;
        std::chrono::time_point<std::chrono::system_clock> m_EndPoint;
    };

}

#endif // !ANUBIS_TIMER_HPP





