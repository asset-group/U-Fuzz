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

#pragma once

//*****************************************************************************
// Includes
//*****************************************************************************

#include <iostream>
#include <utility>
using namespace std;

#include "byte_interpret_common.h"


//*****************************************************************************
// C_reference_counter_ptr ****************************************************
//*****************************************************************************
//! Template class for shared pointers.
//*****************************************************************************
//! Relies on unqualified calls to
//!
//! void  C_reference_counter_ptr_add_ref (T * p);
//! void  C_reference_counter_ptr_release (T * p);
//!
//! You must defines this 2 functions (with the good behavior) for your kind
//!  of object.
//! NB: These 2 functions already exist for classes inherited from
//!  C_reference_counter.
//*****************************************************************************

template <class TYPE> class C_reference_counter_ptr
{
public:

    //---------------------------------------------------------------------------
    //! Constructor.
    //---------------------------------------------------------------------------
    C_reference_counter_ptr(TYPE *p_object)
        : _p_object(p_object)
    {
        add_ref();
    }

    //---------------------------------------------------------------------------
    //! Default constructor.
    //---------------------------------------------------------------------------
    C_reference_counter_ptr() : _p_object(nullptr) {}

    //---------------------------------------------------------------------------
    //! Copy constructor.
    //---------------------------------------------------------------------------
    C_reference_counter_ptr(const C_reference_counter_ptr&  rhs)
        : _p_object(rhs.get())
    {
        add_ref();
    }

    //---------------------------------------------------------------------------
    //! Copy constructor template.
    //! For initialization of a shared pointer from a compatible shared pointer (
    //! e.g. initialization of C_reference_counter_ptr<const T> from
    //! C_reference_counter_ptr<T>).
    //---------------------------------------------------------------------------
    template <class OTHER>
    C_reference_counter_ptr(const C_reference_counter_ptr<OTHER>  & rhs)
        : _p_object(rhs.get())
    {
        add_ref();
    }

    //---------------------------------------------------------------------------
    //! Assignment operator. 
    //---------------------------------------------------------------------------
    C_reference_counter_ptr&   operator= (const C_reference_counter_ptr&  rhs)
    {
        //! set deals with auto assigment.
        set(rhs.get());
        return  *this;
    }

    //---------------------------------------------------------------------------
    //! Destructor.
    //---------------------------------------------------------------------------
    ~C_reference_counter_ptr() { sup_ref(); }

    //---------------------------------------------------------------------------
    //! operator ->.
    //! operator *.
    //---------------------------------------------------------------------------
    TYPE*  operator -> () const { return  _p_object; }
    TYPE&  operator *  () const { return *_p_object; }

    //---------------------------------------------------------------------------
    //! Automatic cast.
    //! BECAREFUL about unexpected implicit conversion.
    //---------------------------------------------------------------------------
    operator TYPE* () const { return  _p_object; }

    //---------------------------------------------------------------------------
    //! Returns true if pointer is NULL.
    //! Not necessary if operator TYPE* is defined.
    //---------------------------------------------------------------------------
    bool  operator! () const
    {
        return  _p_object == nullptr;
    }

    //---------------------------------------------------------------------------
    //! Gets the pointer on the data.
    //---------------------------------------------------------------------------
    TYPE       *get() const { return  _p_object; }

    //---------------------------------------------------------------------------
    //! Swap (permits to avoid add_ref and sup_ref operations).
    //---------------------------------------------------------------------------
    void    swap(C_reference_counter_ptr  & rhs)
    {
        TYPE  * tmp = _p_object;
        _p_object = rhs._p_object;
        rhs._p_object = tmp;
    }


private:

    //---------------------------------------------------------------------------
    //! Sets the pointer on the data.
    //---------------------------------------------------------------------------
    void        set(TYPE  * p_object)
    {
        if (p_object != _p_object)
        {
            // sup_ref ();
            // _p_object = p_object;
            // add_ref ();
            // Previous code is simpler but less performant.

            if (_p_object != nullptr)
            {
                C_reference_counter_ptr_release(_p_object);
                _p_object = p_object;
                add_ref();
            }
            else
            {
                // _p_object == nullptr, so :
                // - C_reference_counter_ptr_release is forbidden
                // - p_object != nullptr
                _p_object = p_object;
                C_reference_counter_ptr_add_ref(_p_object);
            }
        }
    }

    //---------------------------------------------------------------------------
    //! Add a reference on the object.
    //---------------------------------------------------------------------------
    inline
    void    add_ref()
    {
        if (_p_object != nullptr)
            C_reference_counter_ptr_add_ref(_p_object);
    }

    //---------------------------------------------------------------------------
    //! Remove a reference on the object.
    //---------------------------------------------------------------------------
    inline
    void    sup_ref()
    {
        if (_p_object != nullptr)
        {
            C_reference_counter_ptr_release(_p_object);
            _p_object = nullptr;
        }
    }


    //---------------------------------------------------------------------------
    //! Pointed object.
    //---------------------------------------------------------------------------
    TYPE      *_p_object;
};

// ============================================================================
// ASSOCIATED FUNCTIONS
// ============================================================================

template<class T>
void  swap (C_reference_counter_ptr<T>  & lhs,
            C_reference_counter_ptr<T>  & rhs)
{
    lhs.swap(rhs);
}

//! operator<<

#if defined(__GNUC__) &&  (__GNUC__ < 3)

template<class Y> std::ostream & operator<< (std::ostream & os, C_reference_counter_ptr<Y> const & p)
{
    os << p.get();
    return os;
}

#else

template<class E, class T, class Y> std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, C_reference_counter_ptr<Y> const & p)
{
    os << p.get();
    return os;
}

#endif
