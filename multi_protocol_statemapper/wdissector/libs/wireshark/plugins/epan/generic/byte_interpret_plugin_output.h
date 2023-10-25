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

#ifndef BYTE_INTERPRET_PLUGIN_OUTPUT_H
#define BYTE_INTERPRET_PLUGIN_OUTPUT_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "byte_interpret_plugin.h"


//*****************************************************************************
// T_byte_interpret_plugin_output_context
//*****************************************************************************

struct T_byte_interpret_plugin_output_context
{
    const void  * P_null1;
    const void  * P_null2;
    const void  * P_library_name;
    const char  * P_user_data;

#ifdef __cplusplus
    T_byte_interpret_plugin_output_context()
        :P_null1(nullptr),
         P_null2(nullptr),
         P_library_name(nullptr),
         P_user_data(nullptr)
    {
    }
#endif
};

//*****************************************************************************
// byte_interpret_plugin_output_begin *****************************************
//*****************************************************************************
// Called when a new capture is started.
// Called when a new capture file is loaded.
// ...
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_begin(
                const T_byte_interpret_plugin_output_context   * P_context);

typedef void  (* T_byte_interpret_plugin_output_begin_cb)(
                const T_byte_interpret_plugin_output_context   * P_context);

//*****************************************************************************
// byte_interpret_plugin_output_value_integer *********************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_value_integer(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     final_type,
                const long long                                  data_value,
                const char *                                     data_value_str,
                const char *                                     error);

typedef void  (* T_byte_interpret_plugin_output_value_integer_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     final_type,
                const long long                                  data_value,
                const char *                                     data_value_str,
                const char *                                     error);

//*****************************************************************************
// byte_interpret_plugin_output_value_float ***********************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_value_float(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     final_type,
                const double                                     data_value,
                const char *                                     data_value_str,
                const char *                                     error);

typedef void  (* T_byte_interpret_plugin_output_value_float_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     final_type,
                const double                                     data_value,
                const char *                                     data_value_str,
                const char *                                     error);

//*****************************************************************************
// byte_interpret_plugin_output_value_string **********************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_value_string(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     final_type,
                const char *                                     data_value,
                const char *                                     data_value_str,
                const char *                                     error);

typedef void  (* T_byte_interpret_plugin_output_value_string_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     final_type,
                const char *                                     data_value,
                const char *                                     data_value_str,
                const char *                                     error);

//*****************************************************************************
// byte_interpret_plugin_output_raw_data **************************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_raw_data(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const int                                        type_bit_size/*,
                const E_raw_data_type                            raw_data_type*/);

typedef void  (* T_byte_interpret_plugin_output_raw_data_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const int                                        type_bit_size/*,
                const E_raw_data_type                            raw_data_type*/);

//*****************************************************************************
// byte_interpret_plugin_output_group_begin ***********************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_group_begin(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name);

typedef void  (* T_byte_interpret_plugin_output_group_begin_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name);

//*****************************************************************************
// byte_interpret_plugin_output_group_append_text *****************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_group_append_text(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     text);

typedef void  (* T_byte_interpret_plugin_output_group_append_text_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     text);

//*****************************************************************************
// byte_interpret_plugin_output_group_end *************************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_group_end(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name);

typedef void  (* T_byte_interpret_plugin_output_group_end_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name);

//*****************************************************************************
// byte_interpret_plugin_output_error *****************************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_error(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     error);

typedef void  (* T_byte_interpret_plugin_output_error_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     error);

//*****************************************************************************
// byte_interpret_plugin_output_missing_data **********************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_missing_data(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     error);

typedef void  (* T_byte_interpret_plugin_output_missing_data_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     type_name,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     error);

//*****************************************************************************
// byte_interpret_plugin_output_cmd_error *************************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_cmd_error(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     error);

typedef void  (* T_byte_interpret_plugin_output_cmd_error_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     error);



//*****************************************************************************
// byte_interpret_plugin_output_cmd_print *************************************
//*****************************************************************************
EXTERN_C 
LIBRARY_EXPORT
void    byte_interpret_plugin_output_cmd_print(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     text);

typedef void  (* T_byte_interpret_plugin_output_cmd_print_cb)(
                const T_byte_interpret_plugin_output_context   * P_context,
                const void                                     * P_null,
                const char *                                     data_name,
                const char *                                     data_simple_name,
                const char *                                     text);


#endif /* BYTE_INTERPRET_PLUGIN_OUTPUT_H */
