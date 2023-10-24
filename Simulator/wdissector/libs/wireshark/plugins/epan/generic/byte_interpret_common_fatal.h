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
// M_FATAL_IF_ ...
//****************************************************************************

void    fatal_pb (const string  & lhs,
                  const string  & comp,
                  const string  & rhs,
                  const char    * file_name,
                  const size_t    file_line);

#define M_FATAL_IF_FALSE(assertion)                                           \
    if ((assertion) == false)                                                 \
    {                                                                         \
        fatal_pb (#assertion, "is", "false",                                  \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_NE(lhs,rhs)                                                \
    if ((lhs) != rhs)                                                         \
    {                                                                         \
        fatal_pb (#lhs " (" + get_string (lhs) + ") ",                        \
                  "!=",                                                       \
                  #rhs " (" + get_string (rhs) + ") ",                        \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_EQ(lhs,rhs)                                                \
    if ((lhs) == rhs)                                                         \
    {                                                                         \
        fatal_pb (#lhs " (" + get_string (lhs) + ") ",                        \
                  "==",                                                       \
                  #rhs " (" + get_string (rhs) + ") ",                        \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_LT(lhs,rhs)                                                \
    if ((lhs) <  rhs)                                                         \
    {                                                                         \
        fatal_pb (#lhs " (" + get_string (lhs) + ") ",                        \
                  "<",                                                        \
                  #rhs " (" + get_string (rhs) + ") ",                        \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_LE(lhs,rhs)                                                \
    if ((lhs) <= rhs)                                                         \
    {                                                                         \
        fatal_pb (#lhs " (" + get_string (lhs) + ") ",                        \
                  "<=",                                                       \
                  #rhs " (" + get_string (rhs) + ") ",                        \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_GT(lhs,rhs)                                                \
    if ((lhs) >  rhs)                                                         \
    {                                                                         \
        fatal_pb (#lhs " (" + get_string (lhs) + ") ",                        \
                  ">",                                                        \
                  #rhs " (" + get_string (rhs) + ") ",                        \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_GE(lhs,rhs)                                                \
    if ((lhs) >= rhs)                                                         \
    {                                                                         \
        fatal_pb (#lhs " (" + get_string (lhs) + ") ",                        \
                  ">=",                                                       \
                  #rhs " (" + get_string (rhs) + ") ",                        \
                  __FILE__, __LINE__);                                        \
    }

#define M_FATAL_IF_NULL(lhs)   M_FATAL_IF_EQ(lhs, nullptr)

#define M_FATAL_COMMENT(comment)                                              \
    {                                                                         \
        std::ostringstream  M_FATAL_COMMENT_oss;                              \
        M_FATAL_COMMENT_oss << comment;                                       \
        fatal_pb ("FATAL :", "", M_FATAL_COMMENT_oss.str (),                  \
                  __FILE__, __LINE__);                                        \
    }

#define M_ASSERT(assertion)     M_FATAL_IF_FALSE(assertion)
#define M_ASSERT_EQ(lhs,rhs)    M_FATAL_IF_NE(lhs,rhs)
#define M_ASSERT_NE(lhs,rhs)    M_FATAL_IF_EQ(lhs,rhs)

