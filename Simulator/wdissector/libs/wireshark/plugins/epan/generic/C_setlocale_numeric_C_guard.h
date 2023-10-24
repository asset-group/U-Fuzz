/*
 * Copyright 2020 Olivier Aveline <wsgd@free.fr>
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

#ifndef C_SETLOCALE_NUMERIC_C_GUARD
#define C_SETLOCALE_NUMERIC_C_GUARD

//*****************************************************************************
// Includes.
//*****************************************************************************


//*****************************************************************************
// C_setlocale_numeric_C_guard
//*****************************************************************************

class C_setlocale_numeric_C_guard
{
public:
    // save the current locale for LC_NUMERIC (used for numeric input/output, e.g. strtoll)
    // change the locale for LC_NUMERIC (so 0.236 is a valid number)
    C_setlocale_numeric_C_guard();

    // restore the saved locale
    ~C_setlocale_numeric_C_guard();

private:
    char* _locale_saved;
};

#endif
