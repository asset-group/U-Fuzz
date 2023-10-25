/*
 * Copyright 2012-2019 Olivier Aveline <wsgd@free.fr>
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

#ifndef BYTE_INTERPRET_PLUGIN_H
#define BYTE_INTERPRET_PLUGIN_H

//*****************************************************************************
// Includes.
//*****************************************************************************


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    
#endif
#endif

#ifndef LIBRARY_EXPORT
#ifdef WIN32
#define LIBRARY_EXPORT  __declspec(dllexport)
#else
#define LIBRARY_EXPORT
#endif
#endif

//*****************************************************************************
// T_byte_interpret_plugin_output
//*****************************************************************************
#if 0
struct T_byte_interpret_plugin_output
{
    const char  * P_user_data;
};
#endif


#endif /* BYTE_INTERPRET_PLUGIN_H */
