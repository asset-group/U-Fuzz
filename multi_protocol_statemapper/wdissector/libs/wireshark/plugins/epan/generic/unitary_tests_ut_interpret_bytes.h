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

#ifndef UNITARY_TEST_UT_INTERPRET_BYTES_H
#define UNITARY_TEST_UT_INTERPRET_BYTES_H

 //*****************************************************************************
// Includes.
//*****************************************************************************

#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

#include "T_type_definitions.h"
#include "T_interpret_data.h"
#include "byte_interpret_parse.h"
#include "byte_interpret.h"


//*****************************************************************************
// ut_interpret_bytes_init ****************************************************
//*****************************************************************************

void ut_interpret_bytes_init(T_type_definitions& type_definitions);

//*****************************************************************************
// C_ut_interpret_bytes_no_decode_guard ***************************************
//*****************************************************************************

class C_ut_interpret_bytes_no_decode_guard
{
public:
    C_ut_interpret_bytes_no_decode_guard();
    ~C_ut_interpret_bytes_no_decode_guard();
private:
    bool    _S_ut_interpret_bytes_decode_nothing_saved;
};

//*****************************************************************************
// ut_interpret_bytes *********************************************************
//*****************************************************************************

void ut_interpret_bytes (const T_type_definitions  & type_definitions,
                         const T_byte_vector       & in_byte_vector,
                         const string              & in_input_string,
                               T_interpret_data    & interpret_data,
                         const char                * output_expected = nullptr);

//*****************************************************************************
// ut_interpret_bytes *********************************************************
//*****************************************************************************

void ut_interpret_bytes (const T_type_definitions  & type_definitions,
                         const char                * in_hexa_str_param,
                         const string              & in_input_string,
                               T_interpret_data    & interpret_data,
                         const char                * output_expected = nullptr);

//*****************************************************************************
// M_TEST_SIMPLE call ut_interpret_bytes with type_definitions & interpret_data
//*****************************************************************************

#define K_eol  "\n"
#define M_TEST_SIMPLE(BIN_DATA,TO_READ,EXPECTED)    \
    ut_interpret_bytes(type_definitions, BIN_DATA, TO_READ, interpret_data, EXPECTED)



#endif
