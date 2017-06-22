
#pragma once;

namespace CXF
{
    template <typename Mutex>
    class ScopedLock
    {
    private:
        Mutex & mtx;

        ScopedLock(const ScopedLock &);
        void operator=(const ScopedLock &);

    public:
        explicit ScopedLock(Mutex & nmtx)
            :mtx(nmtx)
        {
            mtx.lock();
        }

        ~ScopedLock()
        {
            mtx.unlock();
        }
    };

    class SharedLock
    {

    };
}
