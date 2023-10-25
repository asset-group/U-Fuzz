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

#ifndef C_BYTE_INTERPRET_BUILDER_H
#define C_BYTE_INTERPRET_BUILDER_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
using namespace std;


#include "byte_interpret_builder.h"


//*****************************************************************************
// C_byte_interpret_builder
//*****************************************************************************

class C_byte_interpret_builder
{
public:

    virtual
    ~C_byte_interpret_builder() { }

    //*****************************************************************************
    // NB: in_out_frame_data is AFTER the value
    //*****************************************************************************
    virtual
    void    value(const T_type_definitions  & /* type_definitions */,
                  const T_frame_data        & /* in_out_frame_data */,
                  const T_field_type_name   & /* field_type_name */,
                  const string              & /* data_name */,
                  const string              & /* data_simple_name */,
                  const T_attribute_value   & /* attribute_value */,
                  const string              & /* data_value */,
                  const string              & /* final_type */,
                  const int                   /* type_bit_size */,
                  const bool                  /* is_little_endian */,
                  const bool                  /* error */)
    { }

    //*****************************************************************************
    // NB: in_out_frame_data is AFTER the value
    //*****************************************************************************
    virtual
    void    raw_data(const T_type_definitions  & /* type_definitions */,
                     const T_frame_data        & /* in_out_frame_data */,
                     const T_interpret_data    & /* interpret_data */,
                     const T_field_type_name   & /* field_type_name */,
                     const string              & /* data_name */,
                     const string              & /* data_simple_name */,
                     const int                   /* type_bit_size */,
                     const E_raw_data_type       /* raw_data_type */,
                     const bool                  /* is_decoded_data */)
    { }

    //*****************************************************************************
    // NB: group_end will be called
    //*****************************************************************************
    virtual
    void    group_begin(const T_type_definitions  & /* type_definitions */,
                        const T_frame_data        & /* in_out_frame_data */,
                        const T_field_type_name   & /* field_type_name */,
                        const string              & /* data_name */,
                        const string              & /* data_simple_name */)
    { }

    //*****************************************************************************
    // NB: group_begin has been called
    //*****************************************************************************
    virtual
    void    group_append_text(const T_type_definitions  & /* type_definitions */,
                              const T_frame_data        & /* in_out_frame_data */,
//                              const T_field_type_name   & /* field_type_name */,
                              const string              & /* data_name */,
                              const string              & /* data_simple_name */,
                              const string              & /* text */)
    { }

    //*****************************************************************************
    // NB: group_begin has been called
    //*****************************************************************************
    virtual
    void    group_end(const T_type_definitions  & /* type_definitions */,
                      const T_frame_data        & /* in_out_frame_data */,
                      const T_field_type_name   & /* field_type_name */,
                      const string              & /* data_name */,
                      const string              & /* data_simple_name */,
                      const int                   /* type_bit_size */)
    { }

    //*****************************************************************************
    // 
    //*****************************************************************************
    virtual
    void    error(const T_type_definitions  & /* type_definitions */,
                  const T_frame_data        & /* in_out_frame_data */,
                  const T_field_type_name   & /* field_type_name */,
                  const string              & /* data_name */,
                  const string              & /* data_simple_name */,
                  const string              & /* error */)
    { }

    //*****************************************************************************
    // 
    //*****************************************************************************
    virtual
    void    fatal(const T_type_definitions  & /* type_definitions */,
                  const T_frame_data        & /* in_out_frame_data */,
                  const T_field_type_name   & /* field_type_name */,
                  const string              & /* data_name */,
                  const string              & /* data_simple_name */,
                  const string              & /* error */)
    { }

    //*****************************************************************************
    // 
    //*****************************************************************************
    virtual
    void    missing_data(const T_type_definitions  & /* type_definitions */,
                         const T_frame_data        & /* in_out_frame_data */,
                         const T_interpret_data    & /* interpret_data */,
                         const T_field_type_name   & /* field_type_name */,
                         const string              & /* data_name */,
                         const string              & /* data_simple_name */,
                         const string              & /* error */)
    { }

    //*****************************************************************************
    // 
    //*****************************************************************************
    virtual
    void    cmd_error(const T_type_definitions  & /* type_definitions */,
                      const T_frame_data        & /* in_out_frame_data */,
                      const T_field_type_name   & /* field_type_name */,
                      const string              & /* data_name */,
                      const string              & /* data_simple_name */)
    { }

    //*****************************************************************************
    // 
    //*****************************************************************************
    virtual
    void    cmd_fatal(const T_type_definitions  & /* type_definitions */,
                      const T_frame_data        & /* in_out_frame_data */,
                      const T_field_type_name   & /* field_type_name */,
                      const string              & /* data_name */,
                      const string              & /* data_simple_name */)
    { }

    //*****************************************************************************
    // 
    //*****************************************************************************
    virtual
    void    cmd_print(const T_type_definitions  & /* type_definitions */,
                      const T_frame_data        & /* in_out_frame_data */,
                      const T_field_type_name   & /* field_type_name */,
                      const string              & /* data_name */,
                      const string              & /* data_simple_name */)
    { }

};


#endif /* C_BYTE_INTERPRET_BUILDER_H */
