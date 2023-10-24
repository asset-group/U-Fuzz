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

#pragma once

//*****************************************************************************
// Includes.
//*****************************************************************************

#include "precomp.h"
#include "byte_interpret_common_base.h"


//****************************************************************************
// Traces
//****************************************************************************

#define M_TRACE_ENTER(function_name,OSTREAM_OUTPUT_EXPR)                      \
    C_trace    M_trace(function_name);                                        \
    M_TRACE_base (" ", "Enter", function_name << " " << OSTREAM_OUTPUT_EXPR)

#define M_TRACE_ENTER_NO_LEAVE(function_name,OSTREAM_OUTPUT_EXPR)             \
    M_TRACE_ENTER(function_name,OSTREAM_OUTPUT_EXPR);                         \
    M_trace.leave_trace_done()

#define M_TRACE_LEAVE(OSTREAM_OUTPUT_EXPR)                      \
    M_TRACE_base (" ", "Leave", M_trace.A_function_name << " " << OSTREAM_OUTPUT_EXPR);  \
    M_trace.leave_trace_done()


#define M_TRACE_DEBUG(OSTREAM_OUTPUT_EXPR)                                    \
    M_TRACE_base (" ", "     ", OSTREAM_OUTPUT_EXPR)
#define M_TRACE_INFO(OSTREAM_OUTPUT_EXPR)                                     \
    M_TRACE_base ("i", "     ", OSTREAM_OUTPUT_EXPR)
#define M_TRACE_WARNING(OSTREAM_OUTPUT_EXPR)                                  \
    M_TRACE_print ("W", "     ", OSTREAM_OUTPUT_EXPR)
#define M_TRACE_ERROR(OSTREAM_OUTPUT_EXPR)                                    \
    M_TRACE_print ("E", "     ", OSTREAM_OUTPUT_EXPR)
#define M_TRACE_FATAL(OSTREAM_OUTPUT_EXPR)                                    \
    M_TRACE_print ("F", "     ", OSTREAM_OUTPUT_EXPR)
#define M_TRACE_ASSERT(OSTREAM_OUTPUT_EXPR)                                   \
    M_TRACE_print ("A", "     ", OSTREAM_OUTPUT_EXPR)

#define M_TRACE_base(PREFIX1,PREFIX2,OSTREAM_OUTPUT_EXPR)                     \
    if (C_trace::A_debug_status != E_debug_status_OFF)                        \
    {                                                                         \
        M_TRACE_print (PREFIX1, PREFIX2, OSTREAM_OUTPUT_EXPR);                \
    }

#define M_TRACE_print(PREFIX1,PREFIX2,OSTREAM_OUTPUT_EXPR)                    \
    C_trace::print_beginning_of_trace(get_state_ostream(), PREFIX1, PREFIX2); \
    get_state_ostream() << OSTREAM_OUTPUT_EXPR << endl << flush



enum E_debug_status
{
    E_debug_status_OFF,
    E_debug_status_ON,
    E_debug_status_ON_NO_TIME,
};

void            set_debug(E_debug_status    debug);
E_debug_status  get_debug();

class C_debug_set_temporary
{
public:
    C_debug_set_temporary(E_debug_status  debug);
    C_debug_set_temporary();
    ~C_debug_set_temporary();

    void    set(E_debug_status  debug);
    void    unset();
    void    forget();

private:
    C_debug_set_temporary(const C_debug_set_temporary  &);

    E_debug_status    previous_value;
    bool              value_modified;
};



ostream  & get_state_ostream();
ostream  & set_state_ostream(ostream  & new_state_ostream);


//#define M_TRACE_print(OSTREAM_OUTPUT_EXPR)

struct C_trace
{
    C_trace(const char  * function_name);
    ~C_trace();

    static
    void    print_beginning_of_trace(      ostream  & os,
                                     const char     * prefix1,
                                     const char     * prefix2);

    // Specifiy the leave trace has been done
    void    leave_trace_done();

    const char  * A_function_name;
    bool          A_must_do_leave_trace;


    static E_debug_status   A_debug_status;
};

