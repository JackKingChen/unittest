
#pragma once;

template <typename T>

class scoped_ptr
{
public:
    // Constructor.
    explicit scoped_ptr(T* p = 0)
        : p_(p)
    {
    }

    // Destructor.
    ~scoped_ptr()
    {
        delete p_;
    }

    // Access.
    T* get()
    {
        return p_;
    }

    // Access.
    T* operator->()
    {
        return p_;
    }

    // Dereference.
    T& operator*()
    {
        return *p_;
    }

    // Reset pointer.
    void reset(T* p = 0)
    {
        delete p_;
        p_ = p;
    }

private:
    // Disallow copying and assignment.
    scoped_ptr(const scoped_ptr&);
    scoped_ptr& operator=(const scoped_ptr&);

    T* p_;
};

