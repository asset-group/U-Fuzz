/*
 * Copyright 2005-2020 Olivier Aveline <wsgd@free.fr>
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

#ifndef T_interpret_value_h
#define T_interpret_value_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
#include <vector>

#include "T_attribute_value.h"
#include "byte_interpret_common.h"


//*****************************************************************************
// T_interpret_value
// - value
// - group        a voir 
// - reference
// And flags :
// - local               plus tard
//*****************************************************************************

struct T_interpret_value : public CT_debug_object_counter<T_interpret_value>
{
    enum E_type
    {
        E_type_value,
        E_type_group,           // not used
        E_type_reference,       // out/in_out function parameter
        E_type_msg
    };

    T_interpret_value();
    T_interpret_value(const std::string        & name);
    T_interpret_value(const std::string        & name,
                      const T_attribute_value  & attribute_value);

//    T_interpret_value(const T_interpret_value  & rhs);
//    T_interpret_value &  operator=(const T_interpret_value  & rhs);
//    ~T_interpret_value();

    E_type                     get_type() const             { return  (E_type)A_type; }
    const std::string        & get_name() const             { return  A_name; }
    const T_attribute_value  & get_attribute_value() const  { return  A_attribute_value; }
    int                        get_reference_counter() const{ return  A_reference_counter; }

    void    set_type(const E_type         type)             { A_type = type; }
    void    set_name(const std::string  & name)             { A_name = name; }
    void    set_attribute_value(const T_attribute_value  & attribute_value);

    void    incr_reference_counter()                        { ++A_reference_counter; }
    void    decr_reference_counter()                        { --A_reference_counter; }

private:

    // VCEE2008 & 32bits
    // Size = 2 + 2 +     28 +  96 = 128 bytes

    // VCEE2008 & 64bits
    // Size = 2 + 2 + 4 + 40 + 120 = 168 bytes

    unsigned short       A_reference_counter;
    /*E_type*/short      A_type;                 // short permits to save 4 bytes (32bits)
    std::string          A_name;
    T_attribute_value    A_attribute_value;

    friend void    swap(T_interpret_value  & lhs,
                        T_interpret_value  & rhs);
};

void    swap(T_interpret_value  & lhs,
             T_interpret_value  & rhs);


#endif /* T_interpret_value_h */
