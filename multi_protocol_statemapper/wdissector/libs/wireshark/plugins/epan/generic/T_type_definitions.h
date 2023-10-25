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

#ifndef T_type_definitions_h
#define T_type_definitions_h

//****************************************************************************
// Includes.
//****************************************************************************

#include "C_value.h"
#include "T_expression.h"

#include <string>
#include <map>
#include <vector>

using namespace std;


//****************************************************************************
// Override.
//****************************************************************************

enum E_override
{
    E_override_no,
    E_override_yes
};

//*****************************************************************************
// Identifier syntax.
//*****************************************************************************

bool    is_a_valid_identifier_name (const string   & var_name,
                                    const char       also_authorized = '\0');

#define is_a_valid_short_identifier_name    is_a_valid_identifier_name
#define is_a_valid_short_variable_name      is_a_valid_short_identifier_name
#define is_a_valid_type_name                is_a_valid_short_identifier_name

#define is_a_valid_const_variable_name(NAME)  is_a_valid_identifier_name(NAME, ':')
#define is_a_valid_long_variable_name(NAME)   is_a_valid_identifier_name(NAME, '.')

//****************************************************************************
// Aliases.
//****************************************************************************

// Alias name -> type name to be used instead
typedef map<string,string>    T_map_alias_type;


//****************************************************************************
// Const.
//****************************************************************************

// Const name -> value
typedef map<string,C_value>    T_map_const_value;


//****************************************************************************
// Field constraint.
//****************************************************************************

struct T_field_constraint
{
    C_value        min;
    C_value        max;
    C_value        equal;
};

//****************************************************************************
// Field (for struct ...).
//****************************************************************************

struct T_field_type_name_base
{
    string         orig_type;          // original field type, only for trace

    string         type;
    string         name;

    string         display_name;
    string         filter_name;
    string         extended_name;

    const string &  get_display_name()  const  { return  display_name.empty()  ? name : display_name; }
    const string &  get_filter_name()   const  { return  filter_name.empty()   ? name : filter_name; }
    const string &  get_extended_name() const  { return  extended_name.empty() ? name : extended_name; }

    // only for basic types (int, float, char) and enum
    // NOT for struct ...
    int            basic_type_bit_size;

    // string/raw/subproto/insproto size or switch parameter
    string         str_size_or_parameter;

    //------------------------------------------------------------
    // Pre treatment (raw data extraction)
    //------------------------------------------------------------

    // byte order
    string         str_byte_order;

    // decoder function name
    string         str_decoder_function;

    //------------------------------------------------------------
    // Wireshark specific treatment
    //------------------------------------------------------------

    // subproto/insproto dissector name
    string         str_dissector;

    //------------------------------------------------------------
    // Post treatment
    //------------------------------------------------------------

    // integer/float no_statement
    C_value        no_statement;

    // integer/float transform
    C_value          transform_quantum;
    C_value          transform_offset;
    T_expression     transform_expression;
    C_value::E_type  transform_expression_type;

    // integer/float constraints
    std::vector<T_field_constraint>    constraints;

    // display
    string         str_display;
    string         str_display_expression;

    // return true if no_statement or transform or constraints or display exist
    bool    has_post_treatment() const;

    //------------------------------------------------------------
    //------------------------------------------------------------

    T_field_type_name_base()
        : basic_type_bit_size (-1)
        , transform_expression_type(C_value::E_type_msg)  // not set
    {
    }
};

//****************************************************************************
// Field (for struct ...).
//****************************************************************************

#include "scoped_copyable_ptr.h"
struct T_enum_definition_representation;
struct T_struct_definition;
struct T_switch_definition;
struct T_bitfield_definition;
struct T_field_type_name;

typedef bool    (*T_pf_frame_to_any)
                     (const T_type_definitions    & type_definitions,
                            T_frame_data          & in_out_frame_data,
                            T_interpret_data      & interpret_data,
                      const T_field_type_name     & field_type_name,
                      const string                & data_name,
                      const string                & data_simple_name,
                            ostream               & os_out,
                            ostream               & os_err);

typedef bool    (*T_pf_frame_to_field)(
                         const T_type_definitions    & type_definitions,
                               T_frame_data          & in_out_frame_data,
                               T_interpret_data      & interpret_data,
                         const T_field_type_name     & field_type_name,
                         const string                & data_name,
                         const string                & data_simple_name,
                               ostream               & os_out,
                               ostream               & os_err);

struct T_field_type_name : public T_field_type_name_base
{
    bool                         must_forget;       // field immediately removed (for perf reasons)

    enum E_output_directive
    {
        // Do NOT change values, directly used to change output level.
        E_output_directive_hide = -1,
        E_output_directive_none = 0,
        E_output_directive_show = 1
    };

    E_output_directive           output_directive;

    struct T_array
    {
        enum E_size_type
        {
            E_size_normal,
            E_size_unknow_any,
            E_size_unknow_at_least_1,
        };

        E_size_type        size_type;
        T_expression       size_expression;
    };
    vector<T_array>              str_arrays;

    bool                         A_is_a_variable;
    T_expression                 new_expression;  // expression for variable, set, call
    T_expression                 condition_expression;
    T_expression                 return_expression;
    vector<T_expression>         fct_parameters;
    scoped_copyable_ptr<T_struct_definition>     P_sub_struct;      // struct fields or if/while bloc  
//    vector<T_field_type_name>    sub_struct;      // struct fields or if/while bloc
    vector<T_field_type_name>    sub_struct_2;    // else bloc
    scoped_copyable_ptr<T_bitfield_definition>   P_bitfield_inline;    // inline bitfield
    scoped_copyable_ptr<T_switch_definition>     P_switch_inline;      // inline switch

    //
    bool    is_an_array() const          { return  str_arrays.empty() == false; }
    bool    is_a_variable() const        { return  A_is_a_variable; }

    //
    const T_expression  & get_var_expression() const { return  new_expression; }
    const T_expression  & get_set_expression() const { return  new_expression; }
    const T_expression  & get_condition_expression() const { return  condition_expression; }
    const T_expression  & get_return_expression() const    { return  return_expression; }

    // Because of transform (or ...), the resulting value could change.
    bool    must_force_manage_as_biggest_float() const;
    bool    must_force_manage_as_biggest_int() const;

    // Output directive
    E_output_directive     get_output_directive()    const { return  output_directive; }
    int                    get_output_level_offset() const { return  output_directive; }
    bool    must_hide() const    { return  output_directive == E_output_directive_hide; }
    bool    must_show() const    { return  output_directive == E_output_directive_show; }

    // Field external data.
    int                          wsgd_field_idx;
    // Should be intialized at the end of init.
    // But can also be initialized during interpretation.
    T_pf_frame_to_any                         pf_frame_to_any;
    T_pf_frame_to_field                       pf_frame_to_field;
    const T_enum_definition_representation  * P_type_enum_def;
    const T_struct_definition               * P_type_struct_def;
    const T_switch_definition               * P_type_switch_def;
    const T_bitfield_definition             * P_type_bitfield_def;

    T_field_type_name();
};

ostream &  operator<< (ostream & os, const T_field_type_name  & rhs);

//****************************************************************************
// Struct
//****************************************************************************

// A Struct is mainly an array of fields.
typedef vector<T_field_type_name>                 T_struct_fields;

struct T_struct_definition
{
    T_struct_fields    fields;
    string             printf_args;
#if 1
    // This is absolutely NOT finished due to bug 2402.
    int                field_struct_idx;

    bool               is_a_field_struct() const  { return  field_struct_idx >= 0; }

    T_struct_definition()
        :field_struct_idx(-1)
    {
    }
#endif
};


ostream &  operator<< (ostream & os, const T_struct_definition  & rhs);

// Struct name -> struct definition
typedef map<string,T_struct_definition>           T_map_struct_definition;

//****************************************************************************
// Bitfield
//****************************************************************************

struct T_bitfield_definition
{
    bool                   is_a_bitstream;
    T_field_type_name      master_field;
    T_struct_fields        fields_definition;

    T_bitfield_definition()
        :is_a_bitstream(false)
    {
    }
};

// Bitfield name -> struct definition
typedef map<string,T_bitfield_definition>           T_map_bitfield_definition;

//****************************************************************************
// Enums
//****************************************************************************

struct T_enum_name_val
{
    string       name;     // symbolic name
    long long    value;    // numeric value

    T_enum_name_val () : value (0) {}
};

ostream &  operator<< (ostream & os, const T_enum_name_val  & rhs);

// An Enum is an array of values (symbolic name + numeric value).
typedef vector<T_enum_name_val>                   T_enum_definition;

struct T_enum_definition_representation
{
    T_enum_definition    definition;    // array of values
    size_t               bit_size;      // size in bits
    bool                 is_signed;     // 
    string               representation_type;
    int                  wsgd_enum_values_idx;

    bool    get_integer_value(const string     & symbolic_name,
                                    long long  & integer_value) const;
    const string &  get_symbolic_name(long long    integer_value) const;

    T_enum_definition_representation ()
        : bit_size (0),
          is_signed (false),
          wsgd_enum_values_idx (-1)
    {}
};

ostream &  operator<< (ostream & os,
                 const T_enum_definition_representation  & rhs);

// Enum name -> enum definition representation
typedef map<string,T_enum_definition_representation>    T_map_enum_definition_representation;


//****************************************************************************
// Switchs
//****************************************************************************

struct T_switch_case
{
    T_expression           case_expr;     // expression (if is_switch_expr)
    C_value                case_value;    // value could be string, int, enum symbolic value
    bool                   is_default_case;
    T_struct_fields        fields;        // def

    T_switch_case () : is_default_case (false) {}
};

ostream &  operator<< (ostream        & os,
                 const T_switch_case  & rhs);

// An switch_cases is an array of switch_case values
typedef vector<T_switch_case>                   T_switch_cases;

struct T_switch_definition
{
    bool                 is_switch_expr;       // for switch_expr
    string               case_type;
    T_switch_cases       switch_cases;

    T_switch_definition()
        : is_switch_expr(false)
    {
    }
};

ostream &  operator<< (ostream              & os,
                 const T_switch_definition  & rhs);

// Switch name -> switch definition representation
typedef map<string,T_switch_definition>    T_map_switch_definition;

//****************************************************************************
// Functions
//****************************************************************************

enum E_parameter_direction
{
    E_parameter_in     = 1,
    E_parameter_out    = 2,
    E_parameter_in_out = 3
};

struct T_function_parameter : public T_field_type_name_base
{
    E_parameter_direction  direction;
//    string                 type;
//    string                 name;

    bool                   has_default_value() const    { return  A_has_default_value; }
    const C_value &        get_default_value() const    { return  A_default_value; }
    void                   set_default_value(const C_value  & rhs); 

    T_function_parameter()
        :direction(E_parameter_in),
         A_has_default_value(false)
    {
    }

private:
    bool                   A_has_default_value;
    C_value                A_default_value;
};

void    check_function_parameter_value(
                const T_type_definitions    & type_definitions,
                const T_function_parameter  & function_parameter,
                const C_value               & obj_value);

ostream &  operator<< (ostream               & os,
                 const T_function_parameter  & rhs);

typedef vector<T_function_parameter>                   T_function_parameters;

struct T_function_prototype_definition
{
    string                 return_type;

    const T_function_parameters  & get_function_parameters() const { return  function_parameters; }
    int                    get_nb_of_mandatory_parameters() const  { return  A_nb_of_mandatory_parameters; }
    void                   add_function_parameter(const T_function_parameter  & function_parameter);

    T_function_prototype_definition();

private:
    T_function_parameters  function_parameters;
    int                    A_nb_of_mandatory_parameters;

    friend ostream &  operator<< (ostream                          & os,
                            const T_function_prototype_definition  & rhs);
};

struct T_library_definition;
struct T_library_function_definition;

struct T_function_definition : public T_function_prototype_definition
{
    T_struct_fields        fields;

    T_library_definition  * P_library_def;
    int                     idx_library_function_def;

    T_function_definition()
        : P_library_def(nullptr)
        , idx_library_function_def(-1)
    {
    }
};

ostream &  operator<< (ostream                & os,
                 const T_function_definition  & rhs);

// Function name -> function definition representation
typedef map<string,T_function_definition>    T_map_function_definition;

//****************************************************************************
// Plugin output
//****************************************************************************

#include "byte_interpret_plugin_output.h"

struct T_plugin_output_definition
{
    std::string    library_name;
    std::string    user_data;

    T_byte_interpret_plugin_output_begin_cb              byte_interpret_plugin_output_begin_cb;
    T_byte_interpret_plugin_output_value_integer_cb      byte_interpret_plugin_output_value_integer_cb;
    T_byte_interpret_plugin_output_value_float_cb        byte_interpret_plugin_output_value_float_cb;
    T_byte_interpret_plugin_output_value_string_cb       byte_interpret_plugin_output_value_string_cb;
    T_byte_interpret_plugin_output_raw_data_cb           byte_interpret_plugin_output_raw_data_cb;
    T_byte_interpret_plugin_output_group_begin_cb        byte_interpret_plugin_output_group_begin_cb;
    T_byte_interpret_plugin_output_group_append_text_cb  byte_interpret_plugin_output_group_append_text_cb;
    T_byte_interpret_plugin_output_group_end_cb          byte_interpret_plugin_output_group_end_cb;
    T_byte_interpret_plugin_output_error_cb              byte_interpret_plugin_output_error_cb;
    T_byte_interpret_plugin_output_missing_data_cb       byte_interpret_plugin_output_missing_data_cb;
    T_byte_interpret_plugin_output_cmd_error_cb          byte_interpret_plugin_output_cmd_error_cb;
    T_byte_interpret_plugin_output_cmd_print_cb          byte_interpret_plugin_output_cmd_print_cb;

    T_byte_interpret_plugin_output_context               context;
};

ostream &  operator<< (ostream                     & os,
                 const T_plugin_output_definition  & rhs);

typedef vector<T_plugin_output_definition>    T_vector_plugin_output_definition;

//****************************************************************************
// Library
//****************************************************************************

struct T_library_function_definition
{
    std::string    name;
    void *         funptr;

    T_library_function_definition()
        : funptr(nullptr)
    {
    }
};

struct T_library_definition
{
    std::string    full_name;
    void *         DLLib_handle;

    vector<T_library_function_definition>  library_functions;

    T_library_definition()
        : DLLib_handle(nullptr)
    {
    }
};

ostream &  operator<< (ostream                     & os,
                 const T_library_definition        & rhs);

// library name -> library definition representation
typedef map<string,T_library_definition>    T_map_library_definition;

//****************************************************************************
// T_type_definitions
//****************************************************************************

struct T_type_definitions
{
    enum E_type_kind
    {
        E_type_none,
        E_type_alias,
        E_type_const,
        E_type_struct,
        E_type_bitfield,
        E_type_enum,
        E_type_switch,
        E_type_function
    };

    // trailer size: used only for [*] [+] (*)
    int                                   trailer_sizeof_bits;

    typedef map<string,E_type_kind>     T_map_forward_type;
    T_map_forward_type                    map_forward_type;

    T_map_alias_type                      map_alias_type;
    T_map_const_value                     map_const_value;
    T_map_struct_definition               map_struct_definition;
    T_map_bitfield_definition             map_bitfield_definition;
    T_map_enum_definition_representation  map_enum_definition_representation;
    T_map_switch_definition               map_switch_definition;
    T_map_function_definition             map_function_definition;

    T_vector_plugin_output_definition     vector_plugin_output_definition;
    T_map_library_definition              map_library_definition;

    typedef vector<string>              T_vector_dissector_name;
    T_vector_dissector_name               vector_subdissector_name;

    // 
    void             add_alias(const string      & new_type,
                               const string      & already_existent_type,
                               const E_override    must_override = E_override_no);

    //
    void    set_field_type(T_field_type_name_base  & field,
                     const string             & type_param) const;
    void    set_field_name(T_field_type_name_base  & field,
                     const string             & name_param) const;

    void    set_field_condition_expression(T_field_type_name_base  & field,
                     const string             & name_param) const;

    void    set_field_type_size_or_parameter(T_field_type_name_base  & field,
                               const string             & parameter_param) const;

    void    set_field_no_statement(T_field_type_name_base  & field,
                                  const string             & no_statement_param) const;

    void    set_field_transform_quantum(T_field_type_name_base  & field,
                                  const string             & quantum_param) const;
    void    set_field_transform_offset (T_field_type_name_base  & field,
                                  const string             & offset_param) const;

    void    set_field_transform_expression_integer (T_field_type_name_base  & field,
                                      const string             & expr_param) const;
    void    set_field_transform_expression_float (T_field_type_name_base  & field,
                                      const string             & expr_param) const;

    void    add_field_constraint_min_max(T_field_type_name_base  & field,
                                   const string                  & min_param,
                                   const string                  & max_param) const;
    void    add_field_constraint_min  (T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    add_field_constraint_max  (T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    add_field_constraint_equal(T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    prepend_field_constraint_min_max(T_field_type_name_base  & field,
                                   const string                  & min_param,
                                   const string                  & max_param) const;
    void    prepend_field_constraint_min  (T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    prepend_field_constraint_max  (T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    prepend_field_constraint_equal(T_field_type_name_base  & field,
                                 const string             & val_param) const;

    void    set_field_display           (T_field_type_name_base  & field,
                                   const string             & val_param) const;
    void    set_field_display_expression(T_field_type_name_base  & field,
                                   const string             & val_param) const;

    void    set_field_subdissector      (T_field_type_name_base  & field,
                                   const string             & val_param) const;
    void    add_subdissector(const string             & val_param);

    void    set_field_decoder           (T_field_type_name_base  & field,
                                   const string                  & val_param) const;

    void    set_field_byte_order        (T_field_type_name_base  & field,
                                   const string                  & val_param) const;

    // 
    const T_enum_definition_representation  * get_P_enum    (const string      & type_name) const;
          bool                                is_an_enum    (const string      & type_name) const;

    const T_struct_definition               * get_P_struct  (const string      & type_name) const;
          T_struct_definition               * get_P_struct  (const string      & type_name);
          bool                                is_a_struct   (const string      & type_name) const;

    const T_bitfield_definition             * get_P_bitfield(const string      & type_name) const;
          bool                                is_a_bitfield (const string      & type_name) const;

    const T_switch_definition               & get_switch    (const string      & type_name) const;
    const T_switch_definition               * get_P_switch  (const string      & type_name) const;
          bool                                is_a_switch   (const string      & type_name) const;
          bool                                is_a_switch_value(const string      & type_name) const;
          bool                                is_a_switch_expr (const string      & type_name) const;

    const T_function_definition             & get_function  (const string      & type_name) const;
    const T_function_definition             * get_P_function(const string      & type_name) const;
          bool                                is_a_function (const string      & type_name) const;


    // Enum  : get integer value from symbolic value (<enum type>::<enum symbolic value>).
    // Const : get value 
    // Returns false if nothing found.
    bool    get_type_value (const string   & in_symbolic_name,
                                  C_value  & out_value) const;

    // Search recursively into the map of aliases to find the final type to
    //  be used.
    // If nothing found, returns orig_type.
    const string &   get_final_type (const string  & orig_type) const;

    // NB: returns none for basic types (uint8, float32, string ...).
    E_type_kind    get_defined_type_kind(const string   & type_name) const;
    bool          is_a_defined_type_name(const string   & type_name) const;

    E_type_kind    get_forward_type_kind(const string   & type_name) const;
    bool          is_a_forward_type_name(const string   & type_name) const;

    // returns true for basic types (uint8, float32, string ...).
    bool          is_a_basic_type_name(const string   & type_name) const;

    // returns true for any valid type and forward declaration.
    bool          is_a_type_name(const string   & type_name) const;


    // 
    void             add_forward_declaration(const E_type_kind   type_kind,
                                             const string      & type_name);
                                  
    // Triggers fatal if could NOT define a new type.
    // NB: must be called only when you are going to create the corresponding type.
    // NB: no fatal for basic types (uint8, float32, string ...) !!!
    void    could_define_new_type(const string      & type_name,
                                  const E_type_kind   type_kind,
                                  const E_override    must_override = E_override_no) const;

    T_type_definitions();
};

ostream &  operator<< (ostream                          & os,
                 const T_type_definitions::E_type_kind  & rhs);

ostream &  operator<< (ostream             & os,
                 const T_type_definitions  & rhs);



#endif /* T_type_definitions_h */
