#ifndef __SpinLock_h__
#define __SpinLock_h__

#include "ILock.h"
#include <atomic>

namespace tlib {
    class SpinLock : public ILock {
        std::atomic_flag m_Flag;
    public:
#ifdef WIN32
        SpinLock() {m_Flag._My_flag = 0;}

#elif defined linux
        SpinLock() : m_Flag(ATOMIC_FLAG_INIT) {}
#endif //defined linux

        virtual void Clear() {
            m_Flag.clear(std::memory_order_release);
        }

        virtual void Lock() {
            while (m_Flag.test_and_set(std::memory_order_acquire));
        }

        virtual void UnLock() {
            m_Flag.clear(std::memory_order_release);
        }
    };
}

#endif //__SpinLokc_h__
