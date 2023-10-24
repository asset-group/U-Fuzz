/*
 * Copyright 2005-2020 Olivier Aveline <wsgd@free.fr>
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

#ifndef BYTE_INTERPRET_BUILD_TYPES_READ_TOKEN_H
#define BYTE_INTERPRET_BUILD_TYPES_READ_TOKEN_H

//****************************************************************************
// Includes.
//****************************************************************************

#include <string>
#include <iostream>

using namespace std;


//*****************************************************************************
// read_token_simple_word
// - ignore comments
// - ignore leading spaces
// - manage ;
//*****************************************************************************

bool       read_token_simple_word (
                            istream       & is,
                            string        & str_result);

//*****************************************************************************
// read_token_end_of_statement
// - read ;
//*****************************************************************************

void       read_token_end_of_statement (
                            istream       & is);

//*****************************************************************************
// read_token_simple_word_check_unexpected
// - read a simple word
// - check non existence of unexpected specified char
//*****************************************************************************

bool       read_token_simple_word_check_unexpected (
                            istream       & is,
                            string        & str_result,
                      const char          * P_unexpected_char);

//*****************************************************************************
// read_token_type_simple
// - read simple type (ie without {}()[] ...)
//*****************************************************************************

bool       read_token_type_simple (
                            istream       & is,
                            string        & str_result);

//*****************************************************************************
// read_token_xxx
// simple_word which could be followed by (expression) {expression} [expression] 
// Ne doit pas commencer par l'un des caracteres speciaux
//*****************************************************************************

#define read_token_type_complex  read_token_word_cplx

//*****************************************************************************
// read_token_key_word
// - read a key word (if, set ...)
// - reject {}()[] ...   except "{"
//*****************************************************************************

bool       read_token_key_word (
                            istream       & is,
                            string        & str_result);

//*****************************************************************************
// read_token_key_word_specified
// - read a key word
// - fatal if != key_word_expected
//*****************************************************************************

void       read_token_key_word_specified (
                            istream       & is,
                      const string        & key_word_expected);

//*****************************************************************************
// read_token_key_word_specified
// - read a key word
// - fatal if != one of the key_word_expected
//*****************************************************************************

void       read_token_key_word_specified (
                            istream       & is,
                            string        & str_result,
                      const string        & key_word_expected_1,
                      const string        & key_word_expected_2);
void       read_token_key_word_specified (
                            istream       & is,
                            string        & str_result,
                      const string        & key_word_expected_1,
                      const string        & key_word_expected_2,
                      const string        & key_word_expected_3);

//*****************************************************************************
// read_token_simple_word_or_string
// a simple word or "..." or '...'
//*****************************************************************************

bool       read_token_simple_word_or_string (
                            istream       & is,
                            string        & str_result);

//*****************************************************************************
// read_token_xxx
//*****************************************************************************

// ICIOA pas fini, pas clair


#define read_token_left_any      read_token_type_complex

// right
// take care of : (simple word or "..." or '...' or expression)
// idem read_token_type_complex sauf que ca peut commencer par (

// var_str_array[var_idx+1] + " et quelques ...";
// ->
// var_str_array[var_idx+1] + 
//#define read_token_expression_any(P1,P2)    read_token_word_cplx(P1,P2,K_parser_cfg_expression)
#define read_token_expression_any(P1,P2)    read_token_word_cplx(P1,P2,E_parser_cfg_parameters)

// (...)
#define read_token_expression_parenthesis    read_token_word_cplx

// (...)
#define read_token_parameters    read_token_word_cplx

#define read_token_right_any     read_token_simple_word_or_string

#define read_token_include_name      read_token_simple_word_or_string
#define read_token_case_value        read_token_simple_word_or_string
#define read_token_enum_value        read_token_simple_word_or_string
#define read_token_enum_symbolic     read_token_left_any

//*****************************************************************************
// read_token_parameters_vector
// (param1, ..., paramN)
//*****************************************************************************

bool   read_token_parameters_vector(istream               & is,
                                    vector<string>        & fct_parameters_vector,
                                    const E_parser_cfg      parser_cfg = K_parser_cfg_normal);

//*****************************************************************************
// read_token_function_name
// Accept "" and nil.
//*****************************************************************************

bool       read_token_function_name (
                            istream       & is,
                            string        & str_result);

//*****************************************************************************
// read_token_field_name
// Accept "".
//*****************************************************************************

bool       read_token_field_name (
                            istream       & is,
                            string        & str_result);



#endif /* BYTE_INTERPRET_BUILD_TYPES_READ_TOKEN_H */
