/*
 * Copyright 2008-2020 Olivier Aveline <wsgd@free.fr>
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

#ifndef BYTE_INTERPRET_BUILDER_H
#define BYTE_INTERPRET_BUILDER_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "byte_interpret_common.h"
#include "byte_interpret_build_types.h"
#include "T_frame_data.h"
#include "T_attribute_value.h"
#include "T_interpret_data.h"


class C_byte_interpret_builder;


//*****************************************************************************
// interpret_builder_set_builder **********************************************
//*****************************************************************************

// void    interpret_builder_set_builder(C_byte_interpret_builder  * P_builder);

//*****************************************************************************
// Permits recursive call of generic dissector.
// Mandatory for subdissector and inside dissector.
//*****************************************************************************

class C_interpret_builder_set_temporary
{
public:
    C_interpret_builder_set_temporary(C_byte_interpret_builder  * P_builder);
    ~C_interpret_builder_set_temporary();

    void    set(C_byte_interpret_builder  * P_builder);
    void    unset();
    void    forget();

private:
    C_interpret_builder_set_temporary(const C_interpret_builder_set_temporary  &);

    C_byte_interpret_builder  * previous_value;
    bool                        value_modified;
};


//*****************************************************************************
// interpret_builder_begin
//*****************************************************************************

void    interpret_builder_begin(const T_type_definitions  & type_definitions);

//*****************************************************************************
// interpret_builder_value
//*****************************************************************************

void    interpret_builder_value(const T_type_definitions  & type_definitions,
                                const T_frame_data        & in_out_frame_data,
                                const T_field_type_name   & field_type_name,
                                const string              & data_name,
                                const string              & data_simple_name,
                                const T_attribute_value   & attribute_value,
                                const string              & data_value,
                                const string              & final_type,
                                const int                   type_bit_size,
                                const bool                  is_little_endian,
                                const bool                  error);

//*****************************************************************************
// interpret_builder_raw_data
//*****************************************************************************

enum E_raw_data_type
{
    E_raw_data_any,
    E_raw_data_sub_proto,
    E_raw_data_ins_proto
};
void    interpret_builder_raw_data(const T_type_definitions  & type_definitions,
                                   const T_frame_data        & in_out_frame_data,
                                   const T_interpret_data    & interpret_data,
                                   const T_field_type_name   & field_type_name,
                                   const string              & data_name,
                                   const string              & data_simple_name,
                                   const int                   type_bit_size,
                                   const E_raw_data_type       raw_data_type,
                                   const bool                  is_decoded_data);

//*****************************************************************************
// interpret_builder_group_begin **********************************************
//*****************************************************************************

void    interpret_builder_group_begin(const T_type_definitions  & type_definitions,
                                      const T_frame_data        & in_out_frame_data,
                                      const T_field_type_name   & field_type_name,
                                      const string              & data_name,
                                      const string              & data_simple_name);

//*****************************************************************************
// interpret_builder_group_append_text ****************************************
//*****************************************************************************

void    interpret_builder_group_append_text(const T_type_definitions  & type_definitions,
                                            const T_frame_data        & in_out_frame_data,
//                                            const T_field_type_name   & field_type_name,
                                            const string              & data_name,
                                            const string              & data_simple_name,
                                            const string              & text);

//*****************************************************************************
// interpret_builder_group_end ************************************************
//*****************************************************************************

void    interpret_builder_group_end(const T_type_definitions  & type_definitions,
                                    const T_frame_data        & in_out_frame_data,
                                    const T_field_type_name   & field_type_name,
                                    const string              & data_name,
                                    const string              & data_simple_name,
                                    const int                   type_bit_size);

//*****************************************************************************
// interpret_builder_error ****************************************************
//*****************************************************************************

void    interpret_builder_error    (const T_type_definitions  & type_definitions,
                                    const T_frame_data        & in_out_frame_data,
                                    const T_field_type_name   & field_type_name,
                                    const string              & data_name,
                                    const string              & data_simple_name,
                                    const string              & error);

#define interpret_builder_fatal    interpret_builder_error

//*****************************************************************************
// interpret_builder_missing_data *********************************************
//*****************************************************************************

void    interpret_builder_missing_data
                                   (const T_type_definitions  & type_definitions,
                                    const T_frame_data        & in_out_frame_data,
                                    const T_interpret_data    & interpret_data,
                                    const T_field_type_name   & field_type_name,
                                    const string              & data_name,
                                    const string              & data_simple_name,
                                    const string              & error);

//*****************************************************************************
// interpret_builder_cmd_error ************************************************
//*****************************************************************************

void    interpret_builder_cmd_error(const T_type_definitions  & type_definitions,
                                    const T_frame_data        & in_out_frame_data,
                                    const T_field_type_name   & field_type_name,
                                    const string              & data_name,
                                    const string              & text_to_print);

//*****************************************************************************
// interpret_builder_cmd_fatal
//*****************************************************************************

#define interpret_builder_cmd_fatal    interpret_builder_cmd_error

//*****************************************************************************
// interpret_builder_cmd_print ************************************************
//*****************************************************************************

void    interpret_builder_cmd_print(const T_type_definitions  & type_definitions,
                                    const T_frame_data        & in_out_frame_data,
                                    const T_field_type_name   & field_type_name,
                                    const string              & data_name,
                                    const string              & text_to_print);

//*****************************************************************************
// T_interpret_builder_cmd_print_cb *******************************************
//*****************************************************************************

typedef void  (* T_interpret_builder_cmd_print_cb)(
                                    const T_type_definitions  & type_definitions,
                                    const T_frame_data        & in_out_frame_data,
                                    const T_field_type_name   & field_type_name,
                                    const string              & data_name,
                                    const string              & text_to_print);


#endif /* BYTE_INTERPRET_BUILDER_H */
