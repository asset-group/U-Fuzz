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

#ifndef T_expression_h
#define T_expression_h

//****************************************************************************
// Includes.
//****************************************************************************

#include <string>
#include <vector>

#include "C_value.h"
#include "T_interpret_data.h"
#include "T_frame_data.h"

struct T_type_definitions;


//****************************************************************************
// 
//****************************************************************************
#if 0
expression could be :
  value          integer float string 
  constant       enum
  variable
  - expression
  expression operator expression
  expression ?        expression : expression
  to_string  expression
  to_numeric expression
  print     ( expression, ... )
  function  ( expression, ... )
#endif

//****************************************************************************
// T_expression
//****************************************************************************

struct T_expression : public CT_debug_object_counter<T_expression>
{
    enum E_type
    {
        E_type_none,
        E_type_value,
        E_type_variable,
        E_type_function_call,
        E_type_operation
    };

    enum E_operation
    {
        E_operation_none,
        E_operation_condition,
        E_operation_logical_and,
        E_operation_logical_or,
        E_operation_bit_and,
        E_operation_bit_or,
        E_operation_bit_xor,
        E_operation_bit_shift_left,
        E_operation_bit_shift_right,
        E_operation_equal,
        E_operation_not_equal,
        E_operation_less_or_equal,
        E_operation_greater_or_equal,
        E_operation_less,
        E_operation_greater,
        E_operation_addition,
        E_operation_subtraction,
        E_operation_pow,
        E_operation_multiply,
        E_operation_divide_float,
        E_operation_divide_c,
        E_operation_modulo
    };

    T_expression();

    bool             is_defined() const           { return  A_type != E_type_none; }

    bool             is_a_value() const           { return  A_type == E_type_value; }
    bool             is_a_variable() const        { return  A_type == E_type_variable; }
    const std::string  & get_variable_name() const;

    const std::string  & get_original_string_expression() const   { return  A_original_string_expression; }


    void             build_expression(
                         const T_type_definitions  & type_definitions,
                         const std::string         & str);
    void             build_expression(
                         const C_value             & value);


    void      pre_compute_expression(
                         const T_type_definitions      & type_definitions);


    const C_value &  compute_expression(
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const std::string             & data_name,
                         const std::string             & data_simple_name,
                               std::ostream            & os_out,
                               std::ostream            & os_err) const;

    const C_value &  compute_expression_no_io(
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data) const;

    const C_value &  compute_expression_static(
                         const T_type_definitions      & type_definitions) const;
    
private:

    void             build_expression_str(
                         const T_type_definitions  & type_definitions,
                         const std::string         & str);

    void             build_expression_words(
                         const T_type_definitions        & type_definitions,
                         const std::vector<std::string>  & words);

    void             build_expression_array(
                         const T_type_definitions  & type_definitions,
                         const std::string         & str);

public:
    const C_value &  compute_expression(
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const std::string             & data_name,
                         const std::string             & data_simple_name,
                               std::ostream            & os_out,
                               std::ostream            & os_err,
                               bool                      pre_compute,
                               bool                    & pre_compute_result) const;
private:

    void      compute_expression_variable_array(
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const std::string             & data_name,
                         const std::string             & data_simple_name,
                               std::ostream            & os_out,
                               std::ostream            & os_err,
                               bool                      pre_compute,
                               bool                    & pre_compute_result) const;

    void      compute_expression_function(
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const std::string             & data_name,
                         const std::string             & data_simple_name,
                               std::ostream            & os_out,
                               std::ostream            & os_err,
                               bool                      pre_compute,
                               bool                    & pre_compute_result) const;

    void      compute_expression_operation(
                         const T_type_definitions      & type_definitions,
                               T_interpret_data        & interpret_data,
                               T_frame_data            & in_out_frame_data,
                         const std::string             & data_name,
                         const std::string             & data_simple_name,
                               std::ostream            & os_out,
                               std::ostream            & os_err,
                               bool                      pre_compute,
                               bool                    & pre_compute_result) const;

    // The original string from which the expression have been build.
    //
    std::string                         A_original_string_expression;

    // True if the value has already been calculated (during initialisation).
    mutable bool                        A_value_alreay_computed;

    // Type of expression.
    E_type                              A_type;

    // The value.
    // compute_expression always return this value.
    mutable C_value                     A_value;

    // Name of the variable or name of the function.
    std::string                         A_variable_or_function_name;

    // Operation to do (with A_expressions)
    E_operation                         A_operation;

    // Expression for function call or operation or variable with array
    mutable std::vector<T_expression>   A_expressions;

    // Expressions for array index
#if 0

    struct T_array
    {
        T_expression  expression;
        // reduced_variable_name :
        // if the orginal variable name was   "left[idx_1].field[idx_2][idx_3]"
        //  then the reduced_variable_name is "left[].field[][]"
        // This idx_... gives the position where to insert
        //  the result of the expression.
        int           idx_inside_reduced_variable_name;
    };
//    std::vector<T_array>        A_arrays;
#endif

    friend std::ostream &  operator<< (std::ostream    & os,
                             const T_expression    & rhs);
    friend std::ostream &  operator<< (std::ostream            & os,
                             const T_expression::E_type    & rhs);
    friend std::ostream &  operator<< (std::ostream               & os,
                             const T_expression::E_operation  & rhs);

public:
    const std::vector<T_expression>& get_expressions_for_UT() const { return A_expressions; }
};

std::ostream &  operator<< (std::ostream    & os,
                      const T_expression    & rhs);
std::ostream &  operator<< (std::ostream            & os,
                      const T_expression::E_type    & rhs);
std::ostream &  operator<< (std::ostream               & os,
                      const T_expression::E_operation  & rhs);



#endif /* T_expression_h */
