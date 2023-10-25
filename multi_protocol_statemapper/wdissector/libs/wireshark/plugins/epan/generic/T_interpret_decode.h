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

#ifndef T_interpret_decode_h
#define T_interpret_decode_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
using namespace std;

#include "byte_interpret_common.h"

//*****************************************************************************
// T_interpret_decode
// Contains the decoding function name to use
// Manage the decoding status (in progress or not) 
//*****************************************************************************

struct T_interpret_decode : public CT_debug_object_counter<T_interpret_decode>
{
    void            set_decode_function (string  decode_function);
    const string &  get_decode_function () const                    { return  A_decode_function; }

    void    set_decode_in_progress (bool  in_progress)  { A_decode_in_progress = in_progress; }
    bool     is_decode_in_progress () const             { return  A_decode_in_progress; }

    bool    must_decode () const              { return  A_decode_function != ""; }
    bool    must_decode_now () const          { return  must_decode () && (is_decode_in_progress () == false); }

    T_interpret_decode ()
        :A_decode_in_progress (false)
    {
    }

private:
    string  A_decode_function;
    bool    A_decode_in_progress;
};

//*****************************************************************************
// C_interpret_decode_in_progress
// Permits exception and return (thanks to dtor).
//*****************************************************************************

class C_interpret_decode_in_progress
{
public:
    // Ctor set_decode_in_progress(true)
    C_interpret_decode_in_progress(T_interpret_decode  & interpret_decode);

    // Dtor set_decode_in_progress(false)
    ~C_interpret_decode_in_progress();

private:
    // Copy and assignment are forbidden
    C_interpret_decode_in_progress(const C_interpret_decode_in_progress  &);
    C_interpret_decode_in_progress & operator=(const C_interpret_decode_in_progress  &);

    T_interpret_decode  & A_interpret_decode;
};

//*****************************************************************************
// C_interpret_decode_set_temporary
// Permits recursive call.
// Permits exception and return (thanks to dtor).
//*****************************************************************************

class C_interpret_decode_set_temporary
{
public:
    // Change the decoder function
    C_interpret_decode_set_temporary(T_interpret_decode  & interpret_decode,
                               const std::string         & decoder_function);

    // Restore the previous decoder function
    ~C_interpret_decode_set_temporary();

private:
    // Copy and assignment are forbidden
    C_interpret_decode_set_temporary(const C_interpret_decode_set_temporary  &);
    C_interpret_decode_set_temporary & operator=(const C_interpret_decode_set_temporary  &);

    T_interpret_decode  & A_interpret_decode;
    std::string           A_decoder_function;
    std::string           A_previous_decoder_function;
};



#endif /* T_interpret_decode_h */
