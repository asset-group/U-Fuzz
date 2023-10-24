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

#include "byte_interpret_common.h"


//*****************************************************************************
// C_reference_counter ********************************************************
//*****************************************************************************
// Basic class for objects used with smart pointers.
//*****************************************************************************

class C_reference_counter
{
protected:

    //---------------------------------------------------------------------------
    // Constructor.
    //---------------------------------------------------------------------------
    C_reference_counter() : _ref_counter(0) {}

    //---------------------------------------------------------------------------
    // Copy constructor.
    // Special behavior, DO NOT COPY _ref_counter.
    //---------------------------------------------------------------------------
    C_reference_counter(const C_reference_counter&) : _ref_counter(0) {}

    //---------------------------------------------------------------------------
    // Assignment operator.
    // Special behavior, DO NOT MODIFY _ref_counter.
    //---------------------------------------------------------------------------
    C_reference_counter&  operator= (const C_reference_counter&)
    {
        return *this;
    }

    //---------------------------------------------------------------------------
    // Destructor (verification of reference counter).
    //---------------------------------------------------------------------------
    virtual ~C_reference_counter()
    {
        M_ASSERT(_ref_counter == 0);
    }


private:

    //---------------------------------------------------------------------------
    // Reference counter (number of smart pointers which point on this object).
    // mutable means that a const method can modify the counter.
    // It's normal for this reference counter to be mutable, if not, smart
    //  pointer on a const object is not possible.
    //---------------------------------------------------------------------------
    mutable
    int            _ref_counter;

    //---------------------------------------------------------------------------
    // Increments the reference counter.
    //---------------------------------------------------------------------------
    void           incr_ref() const
    {
        ++_ref_counter;
    }

    //---------------------------------------------------------------------------
    // Decrements the reference counter (destroys itself if necessary).
    //---------------------------------------------------------------------------
    void           decr_ref() const
    {
        if (--_ref_counter <= 0)
            delete this;
    }

    //---------------------------------------------------------------------------
    // Friends.
    //---------------------------------------------------------------------------
    friend void  C_reference_counter_ptr_add_ref(const C_reference_counter *);
    friend void  C_reference_counter_ptr_release(const C_reference_counter *);
};


//-----------------------------------------------------------------------------
// Functions dedicated to C_reference_counter_ptr.
// Only the C_reference_counter_ptr class must use them.
//-----------------------------------------------------------------------------
inline
void  C_reference_counter_ptr_add_ref (const C_reference_counter *  ptr)
{
    ptr->incr_ref ();
}

inline
void  C_reference_counter_ptr_release (const C_reference_counter *  ptr)
{
    ptr->decr_ref ();
}
