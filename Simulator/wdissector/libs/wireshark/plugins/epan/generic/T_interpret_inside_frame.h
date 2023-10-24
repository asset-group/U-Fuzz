/*
 * Copyright 2013-2019 Olivier Aveline <wsgd@free.fr>
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

#ifndef T_interpret_inside_frame_h
#define T_interpret_inside_frame_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
using namespace std;

#include "byte_interpret_common.h"
#include "T_frame_data.h"


//*****************************************************************************
// T_decode_stream_frame ******************************************************
//*****************************************************************************

struct T_decode_stream_frame
{
    T_byte           decoded_data[10000];       // ICIOA hard coded magic number, 1000000 NOT ok for ...shark
    int              decoded_data_bit_size;
    T_frame_data     frame_data;

    T_decode_stream_frame()
        :decoded_data_bit_size(0),
         frame_data(decoded_data, 0, 0)
    {
    }

    // Reset/remove all data
    void            reset();

    // Permits to reset frame_data & decoded_data_size when all data has been read.
    // To avoid decoded_data_bit_size grows indefinitely.
    void            synchronize();

    void            write_n_bytes(const T_byte *  ptr, int  n_bytes);
    void            write_1_byte(T_byte  byte);
    void            write_less_1_byte(T_byte  byte, short  n_bits);
};


//*****************************************************************************
// T_interpret_inside_frame
//*****************************************************************************

struct T_interpret_inside_frame : public CT_debug_object_counter<T_interpret_inside_frame>
{
    // Fatal if AP_decode_stream_frame is nullptr
    T_decode_stream_frame &  get_decode_stream_frame() const;
    T_decode_stream_frame *  get_P_decode_stream_frame() const { return  AP_decode_stream_frame; }

    void                     set_decode_stream_frame(T_decode_stream_frame *  P_rhs);


    T_interpret_inside_frame ()
        :AP_decode_stream_frame (nullptr)
    {
    }

private:
    T_decode_stream_frame *  AP_decode_stream_frame;
};


//*****************************************************************************
// C_decode_stream_frame_set_temporary_if_necessary
//*****************************************************************************

class C_decode_stream_frame_set_temporary_if_necessary
{
public:
    C_decode_stream_frame_set_temporary_if_necessary(T_interpret_inside_frame  & interpret_inside_frame,
                                                     T_decode_stream_frame     & decode_stream_frame);
    ~C_decode_stream_frame_set_temporary_if_necessary();

private:
    C_decode_stream_frame_set_temporary_if_necessary(const C_decode_stream_frame_set_temporary_if_necessary  &);
    C_decode_stream_frame_set_temporary_if_necessary& operator= (const C_decode_stream_frame_set_temporary_if_necessary  &);

    T_interpret_inside_frame  & A_interpret_inside_frame;
    T_decode_stream_frame     & A_decode_stream_frame;
    bool                        A_value_set;
};


#endif /* T_interpret_inside_frame_h */
