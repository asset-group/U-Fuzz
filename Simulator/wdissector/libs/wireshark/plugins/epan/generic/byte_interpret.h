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

#ifndef BYTE_INTERPRET_H
#define BYTE_INTERPRET_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <iostream>
using namespace std;

#include "byte_interpret_common.h"
#include "byte_interpret_build_types.h"
#include "T_interpret_data.h"


//*****************************************************************************
// interpret_bytes ************************************************************
//*****************************************************************************
// type_definitions    : definitions of types (struct, enum, function ...)
// in_out_P_bytes      : binary data to interpret
// in_out_sizeof_bytes : size of binary data, out: size of unread data
// input_stream        : what must be read inside binary data
// os_out              : normal output stream
// os_err              : error  output stream
// interpret_data      : binary data read and interpreted
// Returns true if ok.
//*****************************************************************************
// NB: Returns false if it remains data into inside_frame.
// NB: This interface accepts only bytes and not bits.
//*****************************************************************************

bool    interpret_bytes (const T_type_definitions  & type_definitions,
                         const T_byte             *& in_out_P_bytes,
                               size_t              & in_out_sizeof_bytes,
                               istream             & input_stream,
                               ostream             & os_out,
                               ostream             & os_err,
                               T_interpret_data    & in_out_interpret_data);

bool    interpret_bytes (const T_type_definitions  & type_definitions,
                         const T_byte             *& in_out_P_bytes,
                               size_t              & in_out_sizeof_bytes,
                         const string              & in_input_string,
                               ostream             & os_out,
                               ostream             & os_err,
                               T_interpret_data    & in_out_interpret_data);

//*****************************************************************************
// build_types_and_interpret_bytes ********************************************
//*****************************************************************************
// Idem above except that
//  the definitions of types is directly read inside input_stream
//*****************************************************************************

bool    build_types_and_interpret_bytes (
                  const T_byte             *& in_out_P_bytes,
                        size_t              & in_out_sizeof_bytes,
                        istream             & input_stream,
                        ostream             & os_out,
                        ostream             & os_err);

//*****************************************************************************
// misc
//*****************************************************************************

bool    is_a_switch_value (const T_type_definitions  & type_definitions,
                           const string              & orig_type,
                                 string              & final_simple_type,
                                 string              & discriminant);




#endif /* BYTE_INTERPRET_H */
