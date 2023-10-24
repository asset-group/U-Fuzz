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

#ifndef T_perf_time_h
#define T_perf_time_h

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
#include <iostream>

#ifdef WIN32
#include <sys/timeb.h>
#else
#include <time.h> 
#include <sys/times.h> 
#endif

//*****************************************************************************
// T_perf_time
//*****************************************************************************

class T_perf_time
{
public:
    T_perf_time()
    {
#ifdef WIN32
        ftime (&A_perf_time);
#else
        A_perf_times = times(nullptr);
        A_perf_clock = clock();
#endif
    }

    bool  operator!= (const T_perf_time  & rhs) const;

private:

#ifdef WIN32
    struct timeb  A_perf_time;
#else
    clock_t       A_perf_times;
    clock_t       A_perf_clock;
#endif

    friend std::ostream &  operator<<(std::ostream & os, const T_perf_time  & rhs);
    friend long    perf_time_diff_ms(const T_perf_time  & timeb_val_lhs,
                                     const T_perf_time  & timeb_val_rhs);
};


std::ostream &  operator<<(std::ostream & os, const T_perf_time  & rhs);


long    perf_time_diff_ms(const T_perf_time  & timeb_val_lhs,
                          const T_perf_time  & timeb_val_rhs);

std::string  get_diff_time_ms_str(const long  diff_total_ms);

std::string  perf_time_diff_ms_str(const T_perf_time  & timeb_val_lhs,
                                   const T_perf_time  & timeb_val_rhs);


#endif /* T_perf_time_h */
