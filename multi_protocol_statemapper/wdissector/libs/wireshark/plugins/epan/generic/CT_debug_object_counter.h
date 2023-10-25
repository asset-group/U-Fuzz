/*
 * Copyright 2012-2019 Olivier Aveline <wsgd@free.fr>
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

#ifndef CT_debug_object_counter_H
#define CT_debug_object_counter_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "precomp.h"
#include <string>
#include <typeinfo>


//****************************************************************************
// C_debug_object_counter
//****************************************************************************

// Set this for debug ONLY : big performance issue.
//#define USE_C_debug_object_counter

#ifdef USE_C_debug_object_counter

class C_debug_object_counter
{
public:
    C_debug_object_counter();
    C_debug_object_counter(const C_debug_object_counter  & );
    ~C_debug_object_counter();

    static int    get_nb_of_objects()                    { return A_debug_counter; }

    static std::string  get_debug_string();


    static void    incr(const char * class_name, const int  sizeof_class);
    static void    decr(const char * class_name);

private:
    static int    A_debug_counter;
};

template<class TYPE>
class CT_debug_object_counter
{
public:
    CT_debug_object_counter()
    {
        C_debug_object_counter::incr(typeid(TYPE).name(), sizeof(TYPE));
    }
    CT_debug_object_counter(const CT_debug_object_counter &)
    {
        C_debug_object_counter::incr(typeid(TYPE).name(), sizeof(TYPE));
    }
    ~CT_debug_object_counter()
    {
        C_debug_object_counter::decr(typeid(TYPE).name());
    }
};
#else

class C_debug_object_counter
{
public:
    C_debug_object_counter()                                  {  }
    C_debug_object_counter(const C_debug_object_counter  & )  {  }
    ~C_debug_object_counter()                                 {  }

    static int    get_nb_of_objects()                    { return  0; }

    static std::string  get_debug_string()               { return  ""; }

};

template<class TYPE>
class CT_debug_object_counter
{
public:
    CT_debug_object_counter() {  }
    CT_debug_object_counter(const CT_debug_object_counter &) {  }
    ~CT_debug_object_counter() {  }
};
#endif



#endif /* CT_debug_object_counter_H */
