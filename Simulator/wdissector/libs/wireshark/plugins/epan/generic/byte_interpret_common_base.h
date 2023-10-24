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

#include <cstring>
#include <climits>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#ifdef WIN32
#else
#include <sys/times.h>
#endif

#include "CT_debug_object_counter.h"


//****************************************************************************
// Unused parameter
// http://stackoverflow.com/questions/7090998/portable-unused-parameter-macro-used-on-function-signature-for-c-and-c
//****************************************************************************

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
# define UNUSED(x)
#else
# define UNUSED(x) x
#endif

//****************************************************************************
//
//****************************************************************************

#ifndef LONGLONG_MAX
#define LONGLONG_MAX        ((long long)(~0ULL>>1))
#define LONGLONG_MIN        (-LONGLONG_MAX - 1)
#endif

#ifdef __lynx__
typedef unsigned int      uint;
#endif
#ifdef WIN32
typedef unsigned int      uint;
typedef unsigned int      ulong;
#else
#include <sys/types.h>
#endif

typedef          long long     longlong;
typedef unsigned long long    ulonglong;

typedef unsigned char   T_byte;

#ifdef FRHED
#define endl "\r\n"
#endif

#define M_WHERE  __FILE__, __LINE__

//****************************************************************************
// Misc
//****************************************************************************

enum E_return_code
{
    E_rc_ok,                       // everything is ok
    E_rc_not_integer,              // format ok BUT integer value NOK
    E_rc_not_found,                // format expected NOT found
    E_rc_multiple_value,           // is a mulitple value
    E_rc_not_ok                    // NOT ok
};

#define K_COMMENT_START  '#'

// NB: from foundation/csc_common
#define M_SIZE_TAB(array) (sizeof(array)/sizeof(array[0]))


//****************************************************************************
// ostream << nullptr
//****************************************************************************
std::ostream& operator << (std::ostream& os, std::nullptr_t);

//****************************************************************************
// get_string
//****************************************************************************
template <class TYPE>
//inline
string    get_string (const TYPE&   rhs)
{
  ostringstream    oss;
 
  oss << rhs;

  return  oss.str();
}
#if 1
#include <cstdio>
// moins performant sur le tableau ?
template <>
inline
string    get_string (const long long&   rhs)
{
  char    str_tmp[99+1];
#ifdef WIN32
  sprintf(str_tmp, "%I64d", rhs);
#else
  sprintf(str_tmp, "%lld", rhs);
#endif
  return  str_tmp;
}
#endif
//****************************************************************************
// rfind : reverse find
//****************************************************************************
template <class T_iter,
          class T_value>
inline
T_iter    rfind (const T_iter     begin,
                 const T_iter     end,
                 const T_value  & value)
{
    if (begin == end)
        return  end;

    T_iter  current = end;

    do
    {
        --current;

        if (*current == value)
            return  current;

    } while (current != begin);

    return  end;
}

//*****************************************************************************
// get_files_in_dir ***********************************************************
//*****************************************************************************

int   get_files_in_dir( const string          & dir_name,
                        const string          & begin_file_name,
                        const string          & end_file_name,
                              vector<string>  & file_names,
                        const bool              full_name_required);
