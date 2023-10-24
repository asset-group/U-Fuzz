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

//!@n      Provides functions to print (for debug purpose) data.
//!@n      Add a prefix at the beginning of the output.
//!@n  
//!@n      - print template function which use ostream & operator<<.
//!@n      - M_PRINT_ATTRIBUT to print an object attribute.
//!@n  
//!@n  Important              : 
//!@n      If a class TYPE provides a print method, you must define a print
//!@n       function (for this TYPE) which calls the corresponding print
//!@n       method.

#ifndef F_COMMON_PRINT_H
#define F_COMMON_PRINT_H

//*****************************************************************************
// Includes
//*****************************************************************************

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>

using namespace std;


//*****************************************************************************
//    Name      : print
//!@n Role      : output the in_line_prefix and the specified object on the
//!@n              specified stream.
//!@n             Works for any type which works with ostream & operator<<.
//!@n Important : 
//!@n  To have a well printed format (i.e. in_line_prefix appears at the
//!@n   beginning of EACH line), this function must be redefined for any class
//!@n   which implement a print method.
//!@n
//!@n  ATTENTION, an operator<< on a given TYPE must NOT call the print 
//!@n   function with TYPE parameter if this function has not been redefined.
// Example, the following function implementation :
// ostream &  operator<< (ostream & os, const TYPE & rhs)
// {
//     return  print (os, rhs, "");
// }
//  is FORBIDDEN if ostream & print (ostream &, const TYPE &, const string &)
//  is NOT defined.
//*****************************************************************************
template <class TYPE>
inline
ostream &
print     (      ostream              & in_out_os,
           const TYPE                 & in_right,
           const string               & in_line_prefix)
{
    in_out_os << in_line_prefix;
    in_out_os << in_right;
    in_out_os << endl;
    return  in_out_os;
}


//*****************************************************************************
//    Name      : print_container
//!@n Role      : Print a container with STL compliant interface.
//!@n Important : 
//*****************************************************************************
template <class TYPE>
inline
ostream &
print_container (      ostream       & os,
                 const TYPE          & rhs,
                 const string        & in_line_prefix)
{
    typename TYPE::const_iterator  iter;
    for (  iter  = rhs.begin ();
           iter != rhs.end ();
         ++iter)
    {
        print (os, *iter, in_line_prefix);
    }

    return  os;
}


//*****************************************************************************
//    Name      : print
//!@n Role      : Specialized version for bool.
//!@n Important :
//*****************************************************************************
template <>
inline
ostream &
print (      ostream       & os,
       const bool          & rhs,
       const string        & in_line_prefix)
{
    const char *  bool_str = rhs == false ? "false" : "true";
    return  print (os, bool_str, in_line_prefix);
}


//*****************************************************************************
//    Name      : print
//!@n Role      : Specialized version for vector.
//!@n Important : 
//*****************************************************************************
template <class TYPE>
inline
ostream &
print (      ostream       & os,
       const vector<TYPE>  & rhs,
       const string        & in_line_prefix)
{
    return  print_container (os, rhs, in_line_prefix);
}


//*****************************************************************************
//    Name      : print
//!@n Role      : Specialized version for list.
//!@n Important : 
//*****************************************************************************
template <class TYPE>
inline
ostream &
print (      ostream     & os,
       const list<TYPE>  & rhs,
       const string      & in_line_prefix)
{
    return  print_container (os, rhs, in_line_prefix);
}


//*****************************************************************************
//    Name      : print
//!@n Role      : Specialized version for pair<const KEY, VAL> (map element).
//!@n Important : 
//*****************************************************************************
template <class KEY, class VAL>
inline
ostream &
print (      ostream               & os,
       const pair<const KEY, VAL>  & rhs,
       const string                & in_line_prefix)
{
    ostringstream    oss;
    oss << in_line_prefix << rhs.first << " : ";
    return  print (os, rhs.second , oss.str ());
}



//*****************************************************************************
//    Name      : print
//!@n Role      : Specialized version for map.
//!@n Important : 
//*****************************************************************************
template <class TYPE_ELT,
          class TYPE_DATA>
inline
ostream &
print (      ostream                  & os,
       const map<TYPE_ELT,TYPE_DATA>  & rhs,
       const string                   & in_line_prefix)
{
    return  print_container (os, rhs, in_line_prefix);
}


//*****************************************************************************
//    Name      : M_PRINT_ATTRIBUT
//!@n Role      : Call the print function with a line prefix = 
//!@n              in_line_prefix + <attribut name> " => "
//!@n Important : Used ONLY into a print method.
//*****************************************************************************

#define M_PRINT_ATTRIBUT(in_out_os,ATTRIBUT,in_line_prefix)                   \
    ::print (in_out_os, ATTRIBUT, in_line_prefix + #ATTRIBUT " => ")


//*****************************************************************************
//    Name      : M_PRINT_INSIDE_ATTRIBUT
//!@n Role      : Idem previous for a private type (with print method inside
//!@n              the class itself.
//!@n Important : Used ONLY into a print method.
//*****************************************************************************

#define M_PRINT_INSIDE_ATTRIBUT(in_out_os,ATTRIBUT,in_line_prefix)           \
    print (in_out_os, ATTRIBUT, in_line_prefix + #ATTRIBUT " => ")


#endif /* F_COMMON_PRINT_H */
