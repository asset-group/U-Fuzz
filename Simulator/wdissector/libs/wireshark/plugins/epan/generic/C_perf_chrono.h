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

#ifndef C_PERF_CHRONO_H
#define C_PERF_CHRONO_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "byte_interpret_common.h"
#include "T_interpret_data.h"
#include "T_perf_time.h"


//*****************************************************************************
// C_perf_chrono
//*****************************************************************************

class C_perf_chrono
{
public:
    C_perf_chrono(      T_interpret_data    & interpret_data,
                  const char                * in_input_string,
                  const size_t                in_sizeof_bytes);
    ~C_perf_chrono();

    static void    end();

private:

    static void    output(
        const long              ms_diff_ctor,
        const int               nb_bytes,
        const string          & decode_function,
        const E_debug_status    debug,
        const string          & what);

    T_interpret_data    & A_interpret_data;
    const char  * A_input_string;
    const size_t  A_sizeof_bytes;
    T_perf_time   A_perf_time_ctor;

    static long   A_total_ms;
    static long   A_total_ms_decode;
    static long   A_total_ms_no_decode;
    static long   A_total_sizeof_bytes;
    static long   A_total_sizeof_bytes_decode;
    static long   A_total_sizeof_bytes_no_decode;
};

#endif
