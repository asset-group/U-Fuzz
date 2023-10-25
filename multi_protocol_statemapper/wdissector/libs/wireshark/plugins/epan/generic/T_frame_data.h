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

#ifndef T_frame_data_h
#define T_frame_data_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "byte_interpret_common.h"


//*****************************************************************************
// T_frame_data
// Reference some raw data (data not copied, allocated, deleted).
// Manage a position inside this data.
// Permits to read/write inside data.
//*****************************************************************************

struct T_frame_data : public CT_debug_object_counter<T_frame_data>
{
    T_frame_data();

    // Ctor which define the data used.
    T_frame_data(const void       * initial_P_bytes,
                 const short        bit_offset_in_1st_byte,
                 const long         initial_sizeof_bits);

    // All methods except ...physical... take care of bit_offset_in_1st_byte.
    // E.g: get_bit_offset gives the bit offset STARTING AT bit_offset_in_1st_byte.

    // The offset of the current position from the beginning
    long            get_bit_offset() const            { return  A_initial_sizeof_bits - A_remaining_bits; }
    long            get_physical_bit_offset() const   { return  A_bit_offset_in_1st_byte + get_bit_offset(); }

    // The current position
    const T_byte  * get_P_bytes() const               { return  A_P_bytes; }
    const T_byte  * get_initial_P_bytes() const       { return  A_initial_P_bytes; }

    bool            is_physically_at_beginning_of_byte() const   { return  ((get_physical_bit_offset() % 8) == 0); }

    // The remaining size from the current position
    long            get_remaining_bits() const   { return  A_remaining_bits; }
    long            get_remaining_entire_bytes() const;


    // The initial size
    long      get_initial_sizeof_bits() const    { return  A_initial_sizeof_bits; }


    // Move inside data.
    bool    can_move_1_byte_forward() const             { return  (A_remaining_bits >= 8); }
    void        move_1_byte_forward()                   { move_bit(8); }

    bool    can_move_forward(long    byte_offset, short   bit_offset_in_byte) const;
    void        move_forward(long    byte_offset, short   bit_offset_in_byte);

    bool    can_move_bit_forward(long    bit_offset) const;
    void        move_bit_forward(long    bit_offset);

    bool    can_move(long    byte_offset, short   bit_offset_in_byte) const;
    void        move(long    byte_offset, short   bit_offset_in_byte);

    bool    can_move_bit(long    bit_offset) const;
    void        move_bit(long    bit_offset);

    void    set_offset(long    byte_offset, short   bit_offset_in_byte);
    void    set_bit_offset(long    bit_offset);

    // Read some data.
    T_byte          read_1_byte();
    void            read_n_bytes(short  n_bytes, void  * P_n_bytes_read);
    T_byte          read_less_1_byte(short  n_bits);
    void            read_n_bits(short  n_bits, void  * P_n_bytes_read, short  n_bytes_read);


    // Parent frame
    void         set_initial_frame_starting_bit_offset(long    bit_offset);
    long         get_bit_offset_into_initial_frame() const   { return  get_bit_offset() + A_initial_frame_starting_bit_offset; }
    void         set_bit_offset_into_initial_frame(long    bit_offset);


    // Interface dedicated to T_decode_stream_frame
    void        n_bits_data_appended(long    n_bits) { A_initial_sizeof_bits += n_bits; A_remaining_bits += n_bits; }

private:
protected:
    // NB: copy ctor and assignment operators are ok
    // Because no memory is managed here, it only points to ...

    const T_byte  * A_P_bytes;
    long            A_remaining_bits;

    const T_byte  * A_initial_P_bytes;
    short           A_bit_offset_in_1st_byte;
    long            A_initial_sizeof_bits;

    // Bit offset into the parent frame where this frame starts
    long            A_initial_frame_starting_bit_offset;
};

//*****************************************************************************
// 
//*****************************************************************************
inline
T_byte  bit_erase_left(T_byte  byte, short  n_bits)
{
    byte <<= n_bits;
    byte >>= n_bits;
    return  byte;
}

inline
T_byte  bit_erase_right(T_byte  byte, short  n_bits)
{
    byte >>= n_bits;
    byte <<= n_bits;
    return  byte;
}

inline
T_byte  bit_erase_left_right(T_byte  byte, short  n_bits_left, short  n_bits_right)
{
    byte = bit_erase_left (byte, n_bits_left);
    byte = bit_erase_right(byte, n_bits_right);
    return  byte;
}

inline
T_byte  bit_set_left(T_byte  byte, short  n_bits)
{
    byte |= bit_erase_right(0xff, 8 - n_bits);
    return  byte;
}

inline
T_byte  bit_set_right(T_byte  byte, short  n_bits)
{
    byte |= bit_erase_left(0xff, 8 - n_bits);
    return  byte;
}

inline
T_byte  bit_set_left_right(T_byte  byte, short  n_bits_left, short  n_bits_right)
{
    byte = bit_set_left (byte, n_bits_left);
    byte = bit_set_right(byte, n_bits_right);
    return  byte;
}




#endif /* T_frame_data_h */
