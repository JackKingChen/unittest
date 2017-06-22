
#pragma  once;

#include <assert.h>

namespace CXF
{
    /**
     * Mutex
     */
    class Mutex
    {
    private:
        void* _mutex;

    public:
        /**
         * Default constructor.
         */
        explicit Mutex(void);
    public:
        /**
         * Locks the mutex. Blocks if the mutex is held by another thread.
         */
        void lock(void);

        /**
         * Tries to lock the mutex. Returns false immediately if the mutex is already held by another thread. Returns true if the mutex was successfully locked.
         */
        bool tryLock(void);

        /**
         * Unlocks the mutex so that it can be acquired by other threads.
         */
        void unlock(void);

        /**
         * Destructor.
         */
        ~Mutex(void);
    };

    /**
     * FastMutex
     */
    class FastMutex
    {
    private:
        void* _mutex;

    public:
        /**
         * Default constructor.
         */
        explicit FastMutex(void);
    public:
        /**
         * Locks the mutex. Blocks if the mutex is held by another thread.
         */
        void lock(void);

        /**
         * Tries to lock the mutex. Returns false immediately if the mutex is already held by another thread. Returns true if the mutex was successfully locked.
         */
        bool tryLock(void);

        /**
         * Unlocks the mutex so that it can be acquired by other threads.
         */
        void unlock(void);

        /**
         * Destructor.
         */
        ~FastMutex(void);
    };

    class ShareMutex
    {
    private:
        struct state_data
        {
            unsigned shared_count:11,
            shared_waiting:11,
            exclusive:1,
            upgrade:1,
            exclusive_waiting:7,
            exclusive_waiting_blocked:1;

            friend bool operator==(state_data const& lhs,state_data const& rhs)
            {
                return *reinterpret_cast<unsigned const*>(&lhs)==*reinterpret_cast<unsigned const*>(&rhs);
            }
        };

        template<typename T>
        T interlocked_compare_exchange(T* target,T new_value,T comparand)
        {
            assert(sizeof(T)==sizeof(long));
            long const res=InterlockedCompareExchange(reinterpret_cast<long*>(target),
                *reinterpret_cast<long*>(&new_value),
                *reinterpret_cast<long*>(&comparand));
            return *reinterpret_cast<T const*>(&res);
        }

        enum
        {
            unlock_sem = 0,
            exclusive_sem = 1
        };

        state_data state;
        HANDLE semaphores[2];
        HANDLE upgrade_sem;

        void release_waiters(state_data old_state)
        {
            if(old_state.exclusive_waiting)
            {
                assert(ReleaseSemaphore(semaphores[exclusive_sem],1,0)!=0);
            }

            if(old_state.shared_waiting || old_state.exclusive_waiting)
            {
                assert(ReleaseSemaphore(semaphores[unlock_sem],old_state.shared_waiting + (old_state.exclusive_waiting?1:0),0)!=0);
            }
        }
        void release_shared_waiters(state_data old_state)
        {
            if(old_state.shared_waiting || old_state.exclusive_waiting)
            {
                assert(ReleaseSemaphore(semaphores[unlock_sem],old_state.shared_waiting + (old_state.exclusive_waiting?1:0),0)!=0);
            }
        }

        ShareMutex(const ShareMutex&);
        ShareMutex& operator=(const ShareMutex&);

    public:
        ShareMutex()
        {
            semaphores[unlock_sem]   = CreateSemaphore(0,0,LONG_MAX,0);
            semaphores[exclusive_sem]= CreateSemaphore(0,0,LONG_MAX,0);
            if (!semaphores[exclusive_sem])
            {
                assert(ReleaseSemaphore(semaphores[unlock_sem],LONG_MAX,0)!=0);
            }
            upgrade_sem = CreateSemaphore(0,0,LONG_MAX,0);
            if (!upgrade_sem)
            {
                assert(ReleaseSemaphore(semaphores[unlock_sem],LONG_MAX,0)!=0);
                assert(ReleaseSemaphore(semaphores[exclusive_sem],LONG_MAX,0)!=0);;
            }
            state_data state_={0,0,0,0,0,0};
            state=state_;
        }

        ~ShareMutex()
        {
            CloseHandle(upgrade_sem);
            CloseHandle(semaphores[unlock_sem]);
            CloseHandle(semaphores[exclusive_sem]);
        }

        bool try_lock_shared()
        {
            state_data old_state=state;
            for(;;)
            {
                state_data new_state=old_state;
                if(!new_state.exclusive && !new_state.exclusive_waiting_blocked)
                {
                    ++new_state.shared_count;
                    if(!new_state.shared_count)
                    {
                        return false;
                    }
                }

                state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
                if(current_state==old_state)
                {
                    break;
                }
                old_state=current_state;
            }
            return !(old_state.exclusive| old_state.exclusive_waiting_blocked);
        }
#define  THREAD_USES_DATETIME
        void lock_shared()
        {
#if defined THREAD_USES_DATETIME
            assert(timed_lock_shared(::boost::detail::get_system_time_sentinel()));
#else
            assert(try_lock_shared_until(chrono::steady_clock::now()));
#endif
        }

#if defined THREAD_USES_DATETIME
        template<typename TimeDuration>
        bool timed_lock_shared(TimeDuration const & relative_time)
        {
            return timed_lock_shared(get_system_time()+relative_time);
        }
        bool timed_lock_shared(boost::system_time const& wait_until)
        {
            for(;;)
            {
                state_data old_state=state;
                for(;;)
                {
                    state_data new_state=old_state;
                    if(new_state.exclusive || new_state.exclusive_waiting_blocked)
                    {
                        ++new_state.shared_waiting;
                        if(!new_state.shared_waiting)
                        {
                            boost::throw_exception(boost::lock_error());
                        }
                    }
                    else
                    {
                        ++new_state.shared_count;
                        if(!new_state.shared_count)
                        {
                            boost::throw_exception(boost::lock_error());
                        }
                    }

                    state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
                    if(current_state==old_state)
                    {
                        break;
                    }
                    old_state=current_state;
                }

                if(!(old_state.exclusive| old_state.exclusive_waiting_blocked))
                {
                    return true;
                }

                unsigned long const res=detail::win32::WaitForSingleObjectEx(semaphores[unlock_sem],::boost::detail::get_milliseconds_until(wait_until), 0);
                if(res==detail::win32::timeout)
                {
                    for(;;)
                    {
                        state_data new_state=old_state;
                        if(new_state.exclusive || new_state.exclusive_waiting_blocked)
                        {
                            if(new_state.shared_waiting)
                            {
                                --new_state.shared_waiting;
                            }
                        }
                        else
                        {
                            ++new_state.shared_count;
                            if(!new_state.shared_count)
                            {
                                return false;
                            }
                        }

                        state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
                        if(current_state==old_state)
                        {
                            break;
                        }
                        old_state=current_state;
                    }

                    if(!(old_state.exclusive| old_state.exclusive_waiting_blocked))
                    {
                        return true;
                    }
                    return false;
                }

                assert(res==0);
            }
        }
#endif

#ifdef BOOST_THREAD_USES_CHRONO
        template <class Rep, class Period>
        bool try_lock_shared_for(const chrono::duration<Rep, Period>& rel_time)
        {
            return try_lock_shared_until(chrono::steady_clock::now() + rel_time);
        }
        template <class Clock, class Duration>
        bool try_lock_shared_until(const chrono::time_point<Clock, Duration>& t)
        {
            using namespace chrono;
            system_clock::time_point     s_now = system_clock::now();
            typename Clock::time_point  c_now = Clock::now();
            return try_lock_shared_until(s_now + ceil<system_clock::duration>(t - c_now));
        }
        template <class Duration>
        bool try_lock_shared_until(const chrono::time_point<chrono::system_clock, Duration>& t)
        {
            using namespace chrono;
            typedef time_point<chrono::system_clock, chrono::system_clock::duration> sys_tmpt;
            return try_lock_shared_until(sys_tmpt(chrono::ceil<chrono::system_clock::duration>(t.time_since_epoch())));
        }
        bool try_lock_shared_until(const chrono::time_point<chrono::system_clock, chrono::system_clock::duration>& tp)
        {
            for(;;)
            {
                state_data old_state=state;
                for(;;)
                {
                    state_data new_state=old_state;
                    if(new_state.exclusive || new_state.exclusive_waiting_blocked)
                    {
                        ++new_state.shared_waiting;
                        if(!new_state.shared_waiting)
                        {
                            boost::throw_exception(boost::lock_error());
                        }
                    }
                    else
                    {
                        ++new_state.shared_count;
                        if(!new_state.shared_count)
                        {
                            boost::throw_exception(boost::lock_error());
                        }
                    }

                    state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
                    if(current_state==old_state)
                    {
                        break;
                    }
                    old_state=current_state;
                }

                if(!(old_state.exclusive| old_state.exclusive_waiting_blocked))
                {
                    return true;
                }

                chrono::system_clock::time_point n = chrono::system_clock::now();
                unsigned long res;
                if (tp>n) {
                    chrono::milliseconds rel_time= chrono::ceil<chrono::milliseconds>(tp-n);
                    res=detail::win32::WaitForSingleObjectEx(semaphores[unlock_sem],
                        static_cast<unsigned long>(rel_time.count()), 0);
                } else {
                    res=detail::win32::timeout;
                }
                if(res==detail::win32::timeout)
                {
                    for(;;)
                    {
                        state_data new_state=old_state;
                        if(new_state.exclusive || new_state.exclusive_waiting_blocked)
                        {
                            if(new_state.shared_waiting)
                            {
                                --new_state.shared_waiting;
                            }
                        }
                        else
                        {
                            ++new_state.shared_count;
                            if(!new_state.shared_count)
                            {
                                return false;
                            }
                        }

                        state_data const current_state=interlocked_compare_exchange(&state,new_state,old_state);
                        if(current_state==old_state)
                        {
                            break;
                        }
                        old_state=current_state;
                    }

                    if(!(old_state.exclusive| old_state.exclusive_waiting_blocked))
                    {
                        return true;
                    }
                    return false;
                }

                asset(res==0);
            }
        }
#endif

    };
}

