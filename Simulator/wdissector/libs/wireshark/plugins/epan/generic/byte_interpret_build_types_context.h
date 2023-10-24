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

#ifndef byte_interpret_build_types_context_h
#define byte_interpret_build_types_context_h

//****************************************************************************
// Includes.
//****************************************************************************

#include <string>
#include <iostream>


//****************************************************************************
// 
//****************************************************************************

void         build_types_context_reset();
std::string  build_types_context_where();

//****************************************************************************
// files
//****************************************************************************

void         build_types_context_include_file_open (const std::string  & file_name,
                                                    const istream      & is);
void         build_types_context_include_file_open (const std::string  & file_name);
void         build_types_context_include_file_close(const std::string  & file_name);

//****************************************************************************
// types
//****************************************************************************

void         build_types_context_type_kind_begin(const std::string  & type_kind);
void         build_types_context_type_begin     (const std::string  & type_name);
void         build_types_context_type_end       (const std::string  & type_name);
void         build_types_context_type_kind_end  (const std::string  & type_kind);

//****************************************************************************
// lines
//****************************************************************************

void         build_types_context_line_new (const istream  & is);


#endif /* byte_interpret_build_types_context_h */
