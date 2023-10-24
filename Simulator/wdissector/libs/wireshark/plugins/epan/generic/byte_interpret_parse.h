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

#ifndef BYTE_INTERPRET_PARSE_H
#define BYTE_INTERPRET_PARSE_H

//*****************************************************************************
// Includes.
//*****************************************************************************

#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "byte_interpret_common.h"


//*****************************************************************************
// 
//*****************************************************************************

void    skip_blanks_and_comments (istream  & is);
void    skip_line (istream  & is);
bool    is_istream_empty (istream &         is);

//*****************************************************************************
// 
//*****************************************************************************

void       mod_replace_all (      string  & str_to_modify,
                            const string  & replace_old,
                            const string  & replace_new);

string     get_replace_all (      string    str_copy,
                            const string  & replace_old,
                            const string  & replace_new);

void    remove_word_limits (string  & str,
                      const char      limit_left,
                      const char      limit_right);
void    remove_string_limits (string  & str);
bool    is_separator (const char   c);

//*****************************************************************************
// parser
//*****************************************************************************

#define K_parser_cfg_manage_blanks                1    /* blank = separator */
#define K_parser_cfg_ignore_comments_sharp        2
//#define K_parser_cfg_ignore_comments_C            4
//#define K_parser_cfg_ignore_comments_CPP          8
#define K_parser_cfg_keep_strings                16    /* manage "..." */
#define K_parser_cfg_manage_blocks_open_close    32    /* sauf { isole */
#define K_parser_cfg_manage_separator_C          64    /*  */
#define K_parser_cfg_manage_comma               128    /* , = separator */
#define K_parser_cfg_manage_dot_comma           256    /* ; = separator */

#define K_parser_cfg_common  K_parser_cfg_ignore_comments_sharp | K_parser_cfg_keep_strings | K_parser_cfg_manage_blocks_open_close | K_parser_cfg_manage_dot_comma

enum E_parser_cfg
{
    E_parser_cfg_normal      = K_parser_cfg_common | K_parser_cfg_manage_blanks,
    E_parser_cfg_C           = K_parser_cfg_common | K_parser_cfg_manage_blanks | K_parser_cfg_manage_separator_C,
    E_parser_cfg_parameters  = K_parser_cfg_common | K_parser_cfg_manage_comma,
    E_parser_cfg_expression  = K_parser_cfg_common  // a tester
};

#define K_parser_cfg_normal        E_parser_cfg_normal
#define K_parser_cfg_C             E_parser_cfg_C
#define K_parser_cfg_parameters    E_parser_cfg_parameters
#define K_parser_cfg_expression    E_parser_cfg_expression

bool       read_token_word_cplx (
                            istream       & is,
                            string        & str_result,
                      const E_parser_cfg    parser_cfg = K_parser_cfg_normal);
void    istream_to_words (istream         & is,
                          vector<string>  & words,
                    const E_parser_cfg      parser_cfg = K_parser_cfg_normal);
void    string_to_words (
                    const string          & str,
                          vector<string>  & words,
                    const E_parser_cfg      parser_cfg = K_parser_cfg_normal);

//*****************************************************************************
// binary to frame
//*****************************************************************************

typedef vector<T_byte>  T_byte_vector;


void    string_hexa_to_frame (const string         & str_value_hexa,
                                    T_byte_vector  & frame);
void    istream_hexa_to_frame (istream        & is,
                               T_byte_vector  & frame);
void    istream_hexa_dump_to_frame (
                                 istream        & is,
                           const int              nb_of_first_words_to_ignore,
                           const string         & end_of_hexa,
                                 T_byte_vector  & frame);
void    bin_file_to_frame (const string  & file_name,
                                 T_byte_vector  & frame);

//*****************************************************************************
// get_number
//*****************************************************************************

bool    get_number (const char*   word,
                          long*   P_number);
bool    get_number (const char*        word,
                          long long  & P_number);
bool    get_number (const char*        word,
                          int          base,
                          long long  & number);
bool    get_number (const char*        word,
                          double     & P_number);

//*****************************************************************************
// parse a string
//*****************************************************************************

E_return_code  get_before_separator_after (const string  & str,
                                           const char      separator,
                                                 string  & str_left,
                                                 string  & str_right);
E_return_code  get_before_separator_after (const string  & str,
                                           const string  & separator,
                                                 string  & str_left,
                                                 string  & str_right);
E_return_code    decompose_type_sep_value_sep (
                        const string     orig_type,
                        const char       separator_left,
                        const char       separator_right,
                              string   & left_part,
                              string   & value_str);

E_return_code    decompose_left_sep_middle_sep_right (
                        const string     orig_type,
                        const char       separator_left,
                        const char       separator_right,
                              string   & left_part,
                              string   & middle_part,
                              string   & right_part);

void    promote_printf_string_to_64bits(string   & printf_string);

//*****************************************************************************
// dump hexa
//*****************************************************************************

void    dump_buffer (      ostream &  os,
                     const void *     A_buffer,
                     const long       P_user_length);

#endif /* BYTE_INTERPRET_PARSE_H */
