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

#ifndef T_interpret_data_h
#define T_interpret_data_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "T_interpret_read_values.h"
#include "T_interpret_output.h"
#include "T_interpret_byte_order.h"
#include "T_interpret_decode.h"
#include "T_interpret_inside_frame.h"
#include "T_interpret_chrono.h"


//*****************************************************************************
// T_interpret_data
//*****************************************************************************

struct T_interpret_data :
            public T_interpret_read_values,                   // values (field/variable) read
            public T_interpret_output,                        // output level to hide/show data
            public T_interpret_byte_order,                    // byte order (big/little endian)
            public T_interpret_decode,                        // decoder status
            public T_interpret_inside_frame,                  // used for decoded data or ...
            public T_interpret_chrono,                        // time measurement for chrono command
            public CT_debug_object_counter<T_interpret_data>  // debug only
{
};

typedef C_reference_counter_ptr<T_interpret_data>        T_RCP_interpret_data;
typedef C_reference_counter_ptr<const T_interpret_data>  T_RCP_const_interpret_data;


void    swap(T_interpret_data  & lhs,
             T_interpret_data  & rhs);

//*****************************************************************************
// Permits recursive call of generic dissector.
// Mandatory for subdissector and inside dissector.
//*****************************************************************************
#if 0
class C_interpret_data_set_temporary
{
public:
    C_interpret_data_set_temporary(T_interpret_data  & interpret_data);
    C_interpret_data_set_temporary();
    ~C_interpret_data_set_temporary();

    void    set(T_interpret_data  & interpret_data);
    void    unset();
    void    forget();

private:
    C_interpret_data_set_temporary(const C_interpret_data_set_temporary  &);

    T_interpret_data  * previous_value;
    bool                value_modified;
};

//*****************************************************************************
// get_P_interpret_data
//*****************************************************************************

T_interpret_data  * get_P_interpret_data();

//*****************************************************************************
// get_interpret_data
// FATAL if interpret_data has not been set (with C_interpret_data_set_temporary)
//*****************************************************************************

T_interpret_data  & get_interpret_data();
#endif

#endif /* T_interpret_data_h */
