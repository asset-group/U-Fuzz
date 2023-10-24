/*
 * Copyright 2005-2019 Olivier Aveline <wsgd@free.fr>
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

#ifndef T_interpret_byte_order_h
#define T_interpret_byte_order_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>

#include "byte_interpret_common.h"


//*****************************************************************************
// T_interpret_byte_order
// Manage the current byte order
//*****************************************************************************

struct T_interpret_byte_order : public CT_debug_object_counter<T_interpret_byte_order>
{
    bool    must_invert_bytes() const            { return  A_must_invert_bytes; }

    void    set_big_endian();
    void    set_little_endian();
    void    set_as_host();

    bool    is_little_endian() const;

    void         set_data_order(const std::string  & byte_order);
    std::string  get_data_order() const;

    static
    std::string  get_host_order();
    static
    bool         is_host_byte_order_inverted ()  { return  A_is_host_byte_order_inverted; }

    T_interpret_byte_order ();

private:
    bool    A_must_invert_bytes;

    static bool    A_is_host_byte_order_inverted;
};

//*****************************************************************************
// C_interpret_byte_order_set_temporary
// Temporary change the byte order
// Permits recursive call.
// Permits exception and return (thanks to dtor).
//*****************************************************************************

class C_interpret_byte_order_set_temporary
{
public:
    // Change the byte order
    C_interpret_byte_order_set_temporary(T_interpret_byte_order  & interpret_byte_order,
                                   const std::string             & byte_order);

    // Restore the previous byte order
    ~C_interpret_byte_order_set_temporary();

private:
    // Copy and assignment are forbidden
    C_interpret_byte_order_set_temporary(const C_interpret_byte_order_set_temporary  &);
    C_interpret_byte_order_set_temporary & operator=(const C_interpret_byte_order_set_temporary  &);

    T_interpret_byte_order  & A_interpret_byte_order;
    std::string               A_byte_order;
    bool                      A_previous_byte_order_is_little_endian;
};


#endif /* T_interpret_byte_order_h */
