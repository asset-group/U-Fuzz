/*
 * Copyright 2008-2019 Olivier Aveline <wsgd@free.fr>
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

#ifndef T_attribute_value_h
#define T_attribute_value_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "C_value.h"
#include <string>

#include "byte_interpret_common.h"

//*****************************************************************************
// T_attribute_value
//*****************************************************************************

struct T_attribute_value : public CT_debug_object_counter<T_attribute_value>
{
public:
    const C_value&  get_value() const                { return  transformed; }

    // set value as a transformed value (so original is not modified)
    void            set_value_transformed(const C_value&  value);

    // set value as an original value
    void            set_value_original(const C_value&  value);

    // set value as an original value and reset string format
    void            set_value_original_format_reset(const C_value&  value);

    // Only change the string of the value
    void            change_value_str_only(const string &  new_str);

    // error text (unknow enum value, constraints ...)
    void    set_error(const std::string  & in_error);
    bool    has_error() const                        { return  (P_error != nullptr); }
    const std::string *  get_P_error() const         { return  P_error; }

    void    set_bit_position_offset_size(int  offset, int  size);

    T_attribute_value();
    T_attribute_value(const C_value  & value);

    T_attribute_value(const T_attribute_value  & rhs);
    T_attribute_value&  operator=(const T_attribute_value  & rhs);
    ~T_attribute_value();

    void    reset();
    void    reset(const C_value  & value);

private:
    // VCEE2008 & 32bits
    // Sizeof = 64 + 4 + 28 =  96 bytes

    // VCEE2008 & 64bits
    // Sizeof = 72 + 8 + 40 = 120 bytes

    // enum : symbolic name
    // int  : after quantum,offset
    // int  : after hex, oct ip
    C_value    transformed;

    // error text (unknow enum value, constraints ...)
    // Pointer permits to use less memory (when there is no error)
    std::string   * P_error;

    // the original value read
    std::string     original;

    friend void    swap(T_attribute_value  & lhs,
                        T_attribute_value  & rhs);

    friend std::string    attribute_value_to_string (const T_attribute_value  & attribute_value);
};

void    swap(T_attribute_value  & lhs,
             T_attribute_value  & rhs);


std::string    attribute_value_to_string (const T_attribute_value  & attribute_value);


#endif /* T_attribute_value_h */
