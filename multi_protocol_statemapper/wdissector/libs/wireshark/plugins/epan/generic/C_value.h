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

#ifndef C_VALUE_H
#define C_VALUE_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
#include <vector>

#include "CT_debug_object_counter.h"

// ****************************************************************************
// C_value
// - contains a value of a given type : integer, float, string or msg
// - conversions between types (when possible)
// - arithmetic operations (when possible)
// - logical comparisons (when possible)
// ****************************************************************************

struct C_value : public CT_debug_object_counter<C_value>
{
    enum E_type
    {
        E_type_integer,
        E_type_float,
        E_type_string,
        E_type_msg,        // not a simple type
        E_type_struct      // not a simple type, not used ?
    };

    typedef const void *    T_msg;

    C_value ();
    C_value (const E_type               type);

    C_value (const E_type               type,
                   T_msg                msg);

    // bool will be managed as an integer.
    C_value (const bool                 boolean);

    C_value (const          char        integer);
    C_value (const   signed char        integer);
    C_value (const unsigned char        integer);
    C_value (const   signed short       integer);
    C_value (const unsigned short       integer);
    C_value (const   signed int         integer);
    C_value (const unsigned int         integer);
    C_value (const   signed long        integer);
    C_value (const unsigned long        integer);
    C_value (const   signed long long   integer);
    C_value (const unsigned long long   integer);

    C_value (const float                flt);
    C_value (const double               flt);

    C_value (const std::string        & str);
    C_value (const char               * str);

    // Assignment operator
    C_value &  operator=(const C_value  & rhs);

    // Type of the value
    E_type               get_type () const { return  A_type; }
    bool                 is_numeric () const { return  A_type <= E_type_float; }

    // At this time :
    // Only set the external basic type (uint3, float32, string(7) ...)
    // The type itself is not modified
    void                 set_external_type(const std::string  & external_type);
//    const std::string  & get_external_type ()  const;
    bool                 get_external_type_signed ()  const;
    int                  get_external_type_bit_size ()  const;
    int                  get_external_type_byte_size ()  const;
    void                 set_external_type_bit_size(int    external_type_bit_size);

    // Fatal if internal type does not match.
    long long            get_int ()  const;
    std::size_t          get_int_size_t()  const;  // fatal if value exceeds size_t limits
    int                  get_int_int()  const;     // fatal if value exceeds int limits
    bool                 get_bool () const;     // internal type must be integer
    double               get_flt ()  const;
    const std::string  & get_str ()  const;
    T_msg                get_msg ()  const;

    // Returns the internal string.
    const std::string  & as_string () const;

    // Transform the type of value to a string.
    void                 convert_to_string ()       { A_type = E_type_string; }

    // Transform the type of value to a numeric type.
    // Fatal if not possible.
    void                 convert_to_numeric ();
    void                 convert_to_int (int  base = 0);
    void                 convert_to_float ();

    // Try to apply format to the value.
    // Only str modified.
    void                 format(const std::string  & display);

    // Re-initialise string without format.
    // Nothing done for a string value.
    void                 format_reset();

    static
    C_value              sprintf_values(const std::string           & printf_format,
                                        const std::vector<C_value>  & values_to_print);

    // Only str modified.
    // Type and other values NOT modified.
    void                 set_str(const std::string  & str);

    // Try to promote to numeric type.
    // Returns the new type.
    E_type               promote();

    // Position offset and size (unit = bit).
    void    set_bit_position_offset_size(int  offset, int  size);
    int     get_bit_position_offset() const   { return  A_bit_position_offset; }
    int     get_bit_position_size() const     { return  A_bit_position_size; }

    // Logical operators.
    bool       operator&& (const C_value  & rhs) const;
    bool       operator|| (const C_value  & rhs) const;

    // Comparison operators.
    bool       operator== (const C_value  & rhs) const;
    bool       operator!= (const C_value  & rhs) const;
    bool       operator<= (const C_value  & rhs) const;
    bool       operator>= (const C_value  & rhs) const;
    bool       operator<  (const C_value  & rhs) const;
    bool       operator>  (const C_value  & rhs) const;

    // Bit operators.
    C_value    operator&  (const C_value  & rhs) const;
    C_value    operator|  (const C_value  & rhs) const;
    C_value    operator^  (const C_value  & rhs) const;
    C_value    operator<< (const C_value  & rhs) const;
    C_value    operator>> (const C_value  & rhs) const;

    // Arithmetic operators.
    C_value    operator+  (const C_value  & rhs) const;
    C_value    operator-  (const C_value  & rhs) const;
    C_value    operator*  (const C_value  & rhs) const;
//    C_value    operator/  (const C_value  & rhs) const;
    C_value    operator%  (const C_value  & rhs) const;

    static
    C_value    divide_float(const C_value  & lhs, const C_value  & rhs);
    static
    C_value    divide_c(const C_value  & lhs, const C_value  & rhs);
    static
    C_value    pow(const C_value  & lhs, const C_value  & rhs);

    // OPERATOR= versions.
    C_value &  operator&= (const C_value  & rhs);
    C_value &  operator|= (const C_value  & rhs);
    C_value &  operator^= (const C_value  & rhs);
    C_value &  operator<<= (const C_value  & rhs);
    C_value &  operator>>= (const C_value  & rhs);
    C_value &  operator+= (const C_value  & rhs);
    C_value &  operator-= (const C_value  & rhs);
    C_value &  operator*= (const C_value  & rhs);
//    C_value &  operator/= (const C_value  & rhs);
    C_value &  operator%= (const C_value  & rhs);

    C_value &  this_divide_float(const C_value  & rhs);
    C_value &  this_divide_c(const C_value  & rhs);


private:
    static
    C_value    pow_internal(const C_value  & lhs, const C_value  & rhs);

    // Sizeof std::string = 28 bytes  for VCEE2008 & 32bits
    // Sizeof             = 4 + 4 + 8 + 8 + 28 + 4 + 4 = 60 bytes != 64

    // Sizeof std::string = 40 bytes  for VCEE2008 & 64bits
    // Sizeof             = 4 + 4 + 8 + 8 + 40 + 4 + 4 = 72 bytes

    E_type         A_type;
    int            A_external_type_bit_size__cvtbd;
//    std::string    A_external_type__cvtbd;
    long long      A_integer;
    double         A_flt;
    std::string    A_str;

    int            A_bit_position_offset;
    int            A_bit_position_size;

    friend std::ostream &  operator<< (std::ostream  & os, const C_value        & rhs);
};

std::ostream &  operator<< (std::ostream  & os, const C_value::E_type  type);
std::ostream &  operator<< (std::ostream  & os, const C_value        & rhs);


#endif /* C_VALUE_H */
