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

#ifndef C_BYTE_INTERPRET_WSGD_BUILDER_H
#define C_BYTE_INTERPRET_WSGD_BUILDER_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
#include <vector>
using namespace std;

#include "C_byte_interpret_wsgd_builder_base.h"


//*****************************************************************************
// C_byte_interpret_wsgd_builder
//*****************************************************************************

class C_byte_interpret_wsgd_builder : public C_byte_interpret_wsgd_builder_base
{
public:

    C_byte_interpret_wsgd_builder(int                   proto_idx,
                                  tvbuff_t            * wsgd_tvb,
                                  packet_info         * wsgd_pinfo,
                                  proto_tree          * wsgd_tree,
                                  proto_tree          * wsgd_msg_root_tree);

    //*****************************************************************************
    // NB: in_out_frame_data is AFTER the value
    //*****************************************************************************

    void    value(const T_type_definitions  & type_definitions,
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
    // NB: in_out_frame_data is AFTER the value
    //*****************************************************************************

    void    raw_data(const T_type_definitions  & type_definitions,
                     const T_frame_data        & in_out_frame_data,
                     const T_interpret_data    & interpret_data,
                     const T_field_type_name   & field_type_name,
                     const string              & data_name,
                     const string              & data_simple_name,
                     const int                   type_bit_size,
                     const E_raw_data_type       raw_data_type,
                     const bool                  is_decoded_data);

    //*****************************************************************************
    // NB: group_end will be called
    //*****************************************************************************

    void    group_begin(const T_type_definitions  & type_definitions,
                        const T_frame_data        & in_out_frame_data,
                        const T_field_type_name   & field_type_name,
                        const string              & data_name,
                        const string              & data_simple_name);

    //*****************************************************************************
    // NB: group_begin has been called
    //*****************************************************************************

    void    group_append_text(const T_type_definitions  & type_definitions,
                              const T_frame_data        & in_out_frame_data,
//                              const T_field_type_name   & field_type_name,
                              const string              & data_name,
                              const string              & data_simple_name,
                              const string              & text);

    //*****************************************************************************
    // NB: group_begin has been called
    //*****************************************************************************

    void    group_end(const T_type_definitions  & type_definitions,
                      const T_frame_data        & in_out_frame_data,
                      const T_field_type_name   & field_type_name,
                      const string              & data_name,
                      const string              & data_simple_name,
                      const int                   type_bit_size);

    //*****************************************************************************
    // 
    //*****************************************************************************

    void    error(const T_type_definitions  & type_definitions,
                  const T_frame_data        & in_out_frame_data,
                  const T_field_type_name   & field_type_name,
                  const string              & data_name,
                  const string              & data_simple_name,
                  const string              & error);

    //*****************************************************************************
    // 
    //*****************************************************************************
#if 0
    void    fatal(const T_type_definitions  & type_definitions,
                  const T_frame_data        & in_out_frame_data,
                  const T_field_type_name   & field_type_name,
                  const string              & data_name,
                  const string              & data_simple_name,
                  const string              & error);
#endif

    //*****************************************************************************
    // 
    //*****************************************************************************

    void    cmd_error(const T_type_definitions  & type_definitions,
                      const T_frame_data        & in_out_frame_data,
                      const T_field_type_name   & field_type_name,
                      const string              & data_name,
                      const string              & text_to_print);

    //*****************************************************************************
    // 
    //*****************************************************************************

    //*****************************************************************************
    // 
    //*****************************************************************************

    void    cmd_print(const T_type_definitions  & type_definitions,
                      const T_frame_data        & in_out_frame_data,
                      const T_field_type_name   & field_type_name,
                      const string              & data_name,
                      const string              & text_to_print);


private:

    struct T_wsgd_group_data
    {
        proto_tree        * tree;
        proto_item        * item;

        T_wsgd_group_data (proto_tree  * proto_tree_param)
            :tree (proto_tree_param),
             item (nullptr)
        { }
    };

    vector<T_wsgd_group_data>    A_wsgd_group_data;
};


#endif /* C_BYTE_INTERPRET_WSGD_BUILDER_H */
