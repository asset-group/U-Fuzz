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
#include "byte_interpret_common_exception.h"


//****************************************************************************
// Exceptions
//****************************************************************************

enum E_byte_interpret_exception
{
    E_byte_interpret_exception_missing_data,
    E_byte_interpret_exception_loop_deep_break,
    E_byte_interpret_exception_loop_deep_continue,
    E_byte_interpret_exception_loop_break,
    E_byte_interpret_exception_loop_continue,
    E_byte_interpret_exception_return,
    E_byte_interpret_exception_fatal
};

class C_byte_interpret_exception : public std::exception
{
public:
    C_byte_interpret_exception(
                         const char                        * file_name,
                               int                           file_line,
                               E_byte_interpret_exception    bie,
                         const std::string                 & str);
    ~C_byte_interpret_exception() throw() { }

    // override
    virtual const char* what() const throw();
    
    E_byte_interpret_exception    get_cause() const          { return  A_bie; }
    const std::string             get_explanation() const;

private:
    std::string                   A_file_name;
    int                           A_file_line;
    E_byte_interpret_exception    A_bie;
    std::string                   A_str;
    std::string                   A_explanation;
};

