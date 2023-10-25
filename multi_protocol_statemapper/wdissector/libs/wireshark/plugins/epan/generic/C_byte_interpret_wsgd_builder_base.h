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

#ifndef C_BYTE_INTERPRET_WSGD_BUILDER_BASE_H
#define C_BYTE_INTERPRET_WSGD_BUILDER_BASE_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
using namespace std;

#include "C_byte_interpret_builder.h"
#include "generic.h"



proto_item  * cpp_dissect_generic_add_item(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error);

proto_item  * cpp_dissect_generic_add_item_string(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const char            * value);

proto_item  * cpp_dissect_generic_add_item_float(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const float             value);

proto_item  * cpp_dissect_generic_add_item_double(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const double            value);

proto_item  * cpp_dissect_generic_add_item_uint32(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const long long         value);

proto_item  * cpp_dissect_generic_add_item_int32(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const long long         value);

proto_item  * cpp_dissect_generic_add_item_uint64(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const long long         value);

proto_item  * cpp_dissect_generic_add_item_int64(
                                        const int               proto_idx,
                                              tvbuff_t        * tvb,
                                              packet_info     * pinfo,
                                              proto_tree      * tree,
                                        const int               field_idx,
                                        const int               offset,
                                        const int               field_byte_size,
                                        const int               little_endian,
                                        const char            * text,
                                        const int               error,
                                        const long long         value);

//*****************************************************************************
// C_byte_interpret_wsgd_builder_base
//*****************************************************************************

class C_byte_interpret_wsgd_builder_base : public C_byte_interpret_builder
{
public:

    C_byte_interpret_wsgd_builder_base(int                   proto_idx,
                                       tvbuff_t            * wsgd_tvb,
                                       packet_info         * wsgd_pinfo,
                                       proto_tree          * wsgd_tree,
                                       proto_tree          * wsgd_msg_root_tree);

    //*****************************************************************************
    // is_input_data_complete :
    // - true  means all the input data is here, could NOT have more
    // - false means it could have more input data BUT it is not sure
    //*****************************************************************************
    void    set_is_input_data_complete(bool    is_input_data_complete);
    bool    is_input_data_complete() const;

    //*****************************************************************************
    // 
    //*****************************************************************************

    void    missing_data(const T_type_definitions  & type_definitions,
                         const T_frame_data        & in_out_frame_data,
                         const T_interpret_data    & interpret_data,
                         const T_field_type_name   & field_type_name,
                         const string              & data_name,
                         const string              & data_simple_name,
                         const string              & error);


protected:

    struct T_interpret_wsgd
    {
        int                   proto_idx;
        tvbuff_t            * wsgd_tvb;
        packet_info         * wsgd_pinfo;
        proto_tree          * wsgd_tree;
        proto_tree          * wsgd_msg_root_tree;

        T_interpret_wsgd ()
            :proto_idx (-1),
             wsgd_tvb (nullptr),
             wsgd_pinfo (nullptr),
             wsgd_tree (nullptr),
             wsgd_msg_root_tree (nullptr)
        {
        }
    };

    T_interpret_wsgd             A_interpret_wsgd;
    bool                         A_is_input_data_complete;
};


#endif /* C_BYTE_INTERPRET_WSGD_BUILDER_BASE_H */
