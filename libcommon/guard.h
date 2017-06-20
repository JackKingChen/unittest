
#pragma once;

template <typename Mutex> //!< \tparam Mutex (platform-specific) mutex class.
class guard
{ //! Locks the mutex, binding guard<Mutex> to Mutex.
	/*! Example:
	Given a (platform-specific) mutex class, we can wrap code as follows:

	extern mutex global_lock;

	static void f()
	{
		boost::details::pool::guard<mutex> g(global_lock);
		// g's constructor locks "global_lock"

		... // do anything:
				//   throw exceptions
				//   return
				//   or just fall through
	} // g's destructor unlocks "global_lock"
	*/
private:
    Mutex & mtx;

    guard(const guard &); //!< Guards the mutex, ensuring unlocked on destruction, even if exception is thrown.
    void operator=(const guard &);

    public:
    explicit guard(Mutex & nmtx)
    :mtx(nmtx)
    { //! Locks the mutex of the guard class.
        mtx.lock();
    }

    ~guard()
    { //! destructor unlocks the mutex of the guard class.
        mtx.unlock();
    }
}
