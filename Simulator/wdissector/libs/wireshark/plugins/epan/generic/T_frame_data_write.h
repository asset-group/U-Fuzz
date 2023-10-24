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

#ifndef T_frame_data_write_h
#define T_frame_data_write_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "T_frame_data.h"


//*****************************************************************************
// T_frame_data
// Reference some raw data (data not copied, allocated, deleted).
// Manage a position inside this data.
// Permits to read/write inside data.
//*****************************************************************************

struct T_frame_data_write : public T_frame_data
{
    T_frame_data_write();

    // Ctor which define the data used.
    T_frame_data_write(void       * initial_P_bytes,
                 const short        bit_offset_in_1st_byte,
                 const long         initial_sizeof_bits);

    // Write some data.
    void            write_1_byte(T_byte  byte);
    void            write_n_bytes(short  n_bytes, const void  * P_n_bytes_write);
    void            write_less_1_byte(T_byte  byte, short  n_bits);
    void            write_n_bits(short  n_bits, const void  * P_n_bytes_write, short  n_bytes_write);
};


#endif /* T_frame_data_write_h */
