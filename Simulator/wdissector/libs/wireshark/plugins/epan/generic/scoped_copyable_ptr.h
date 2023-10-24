/*
 * Copyright 2009-2019 Olivier Aveline <wsgd@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef scoped_copyable_ptr_h
#define scoped_copyable_ptr_h

//*****************************************************************************
// scoped_copyable_ptr mimics a built-in pointer except that
//  it takes care of :
// - deletion (destructor, operator=, reset ...)
// - copy, assign = deep copy 
//
// This is NOT a shared pointer.
//
// Note: initial code from Boost (http://www.boost.org) scoped_ptr
//       Boost scoped_ptr is NOT copyable
// The reason for which Boost does NOT provide a copyable pointer is (I guess)
//  that this kind of pointer is dangerous due to the slicing problem.
//
// And this version has been designed only for a little usage scope.
//
// Do not use <scoped_copyable_ptr object>.get() as parameter of
//  a scoped_copyable_ptr method expecting a simple pointer.
// -> no deep copy, 2 objects have the same pointer !!!
//
// So it must be used very carefully.
//*****************************************************************************

template<class T> class scoped_copyable_ptr
{
private:

    T * ptr;

    typedef scoped_copyable_ptr<T> this_type;

    void operator==( scoped_copyable_ptr const& ) const;
    void operator!=( scoped_copyable_ptr const& ) const;

public:

    typedef T element_type;

    // Do not use <scoped_copyable_ptr object>.get() as parameter !!!
    explicit scoped_copyable_ptr(T * p = nullptr): ptr(p) // never throws
    {
    }

    // I do not use auto_ptr
    // Not accepted by gcc 4.4.1
#if 0
    explicit scoped_copyable_ptr(std::auto_ptr<T> p): ptr(p.release()) // never throws
    {
    }
#endif

    explicit scoped_copyable_ptr(scoped_copyable_ptr const & rhs)
        :ptr(nullptr)
    {
        if (rhs.ptr != nullptr)
        {
            ptr = new T(*rhs.ptr);
        }
    }

    scoped_copyable_ptr & operator=(scoped_copyable_ptr const & rhs)
    {
        if (ptr != rhs.ptr)
        {
            delete  ptr;
            ptr = new T(*rhs.ptr);
        }
        return  *this;
    }
#if 0
    // Dangereux ?
    scoped_copyable_ptr & operator=(T * p)
    {
        if (ptr != p)
        {
            delete  ptr;
            ptr = p;
        }
        return  *this;
    }
#endif
    ~scoped_copyable_ptr() // never throws
    {
        delete  ptr;
    }

    // Do not use <scoped_copyable_ptr object>.get() as parameter !!!
    void reset(T * p = nullptr) // never throws
    {
        if (p != ptr)
        {
            delete  ptr;
            ptr = p;
        }
    }

    T & operator*() const // never throws
    {
        return *ptr;
    }

    T * operator->() const // never throws
    {
        return ptr;
    }

    T * get() const // never throws
    {
        return ptr;
    }

    bool operator! () const // never throws
    {
        return ptr == nullptr;
    }

    operator bool () const
    {
        return ptr != nullptr;
    }

    void swap(scoped_copyable_ptr & b) // never throws
    {
        T * tmp = b.ptr;
        b.ptr = ptr;
        ptr = tmp;
    }

    bool operator==(nullptr_t) const
    {
        return ptr == nullptr;
    }
    bool operator!=(nullptr_t) const
    {
        return ptr != nullptr;
    }
};

template<class T> inline void swap(scoped_copyable_ptr<T> & a, scoped_copyable_ptr<T> & b) // never throws
{
    a.swap(b);
}

// get_pointer(p) is a generic way to say p.get()

template<class T> inline T * get_pointer(scoped_copyable_ptr<T> const & p)
{
    return p.get();
}

#endif
