
#pragma once;

# include <windows.h>

class atomic_count
{
public:
    explicit atomic_count( long v ): value_( v )
    {
    }

    long operator++()
    {
        return InterlockedIncrement( &value_ );
    }

    long operator--()
    {
        return InterlockedDecrement( &value_ );
    }

    operator long() const
    {
        return static_cast<long const volatile &>( value_ );
    }

private:

    atomic_count( atomic_count const & );
    atomic_count & operator=( atomic_count const & );

    long value_;
};
