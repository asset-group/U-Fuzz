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

#ifndef T_interpret_chrono_h
#define T_interpret_chrono_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>

#include "T_perf_time.h"
#include "byte_interpret_common.h"


//*****************************************************************************
// T_interpret_chrono
// For chrono command (perf debug only)
//*****************************************************************************

struct T_interpret_chrono : public CT_debug_object_counter<T_interpret_chrono>
{
    std::string  compute_chrono_value_from_command(const std::string  & command);

    T_interpret_chrono ();

private:

    static long    A_ms_cumulative;
    T_perf_time    A_timeb_ctor;
    T_perf_time    A_timeb_last;
};

#endif /* T_interpret_chrono_h */
