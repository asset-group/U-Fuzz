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

#ifndef BYTE_INTERPRET_compute_expression_H
#define BYTE_INTERPRET_compute_expression_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>

#include "byte_interpret_common.h"
#include "C_value.h"
#include "T_type_definitions.h"
#include "T_interpret_data.h"
#include "T_frame_data.h"


//*****************************************************************************
// get_complex_value **********************************************************
//*****************************************************************************

E_return_code    get_complex_value (const T_type_definitions  & type_definitions,
                                    const T_interpret_data    & interpret_data,
                                    const std::string         & value_str,
                                          C_value             & value);

//*****************************************************************************
// compute_expressions_in_array ***********************************************
//*****************************************************************************
// Eg : "an_array[val+1][val+2]" -> "an_array[11][12]" (if val=10)
//*****************************************************************************

string    compute_expressions_in_array (
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const string                  & in_str,
                         const string                  & data_name,
                         const string                  & data_simple_name,
                               ostream                 & os_out,
                               ostream                 & os_err);

//*****************************************************************************
// compute_expression *********************************************************
//*****************************************************************************

C_value    compute_expression (
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const std::string             & str,
                         const std::string             & data_name,
                         const std::string             & data_simple_name,
                               std::ostream            & os_out,
                               std::ostream            & os_err);

//*****************************************************************************
// compute_expression_no_io ***************************************************
//*****************************************************************************
// Read no field (do not have in_out_frame_data parameter).
// Variables must be hidden (do not have data_name, os_out, ...).
//*****************************************************************************

C_value    compute_expression_no_io (
                         const T_type_definitions  & type_definitions,
                               T_interpret_data    & interpret_data,
                         const std::string         & str);

//*****************************************************************************
// compute_expression_static **************************************************
//*****************************************************************************
// Static expression (without any attribute/variable)
//*****************************************************************************

C_value    compute_expression_static (const T_type_definitions  & type_definitions,
                                      const std::string         & str);



#endif /* BYTE_INTERPRET_compute_expression_H */
