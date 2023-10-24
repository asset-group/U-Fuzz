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

#ifndef T_type_definitions_build_interface_h
#define T_type_definitions_build_interface_h

//****************************************************************************
// Includes.
//****************************************************************************

NOT USED

//****************************************************************************
// Global
//****************************************************************************

struct T_type_definitions;
struct T_struct_fields;
struct T_field_type_name;

//****************************************************************************
// Aliases.
//****************************************************************************

void    type_definitions_add_alias(T_type_definitions  * P_type_definitions, 
                                   const char          * new_type_name, 
                                   const char          * already_existent_type);

//****************************************************************************
// Field (for struct ...).
//****************************************************************************

struct T_field_type_name_base
{
    string         orig_type;          // original field type, only for trace

    string         type;
    string         name;

    // string/raw/subproto/insproto size or switch parameter
    string         str_size_or_parameter;

    // integer/float
    C_value        transform_quantum;
    C_value        transform_offset;

    C_value        constraint_min;
    C_value        constraint_max;
    C_value        constraint_equal;

    string         str_display;
    string         str_display_expression;
};

//****************************************************************************
// Field (for struct ...).
//****************************************************************************

#include "scoped_copyable_ptr.h"
struct T_struct_definition;
struct T_bitfield_definition;

struct T_field_type_name : public T_field_type_name_base
{
#if 0
    // Qu'est-ce que ca change ?
    // -> interpretation lors du build (donc check)
    // -> gain de vitesse a l'interpretation ? combien ? comment est-ce que je peux mesurer ?
    // -> meilleur code car centralisation du type
    // -> code moins ouvert car centralisation du type
    // -> code suivant la "classe" :
    //    - print (print, debug_print, error, fatal)
    //    - donnee
    //    - ???
    // -> implique un nouveau champ
    enum E_type
    {
        K_type_cmd    = 0x1000,
        K_type_print  = 0x2000,
        K_type_ctrl   = 0x4000,
        K_type_data   = 0x8000,
        K_type_field  = 0x10000,

        E_type_cmd_debug_print          = K_type_cmd & K_type_print + 1,
        E_type_cmd_print                ,
        E_type_cmd_error                ,
        E_type_cmd_fatal                ,
        E_type_cmd_output               = K_type_cmd + 1,
        E_type_cmd_byte_order           ,
        E_type_cmd_save_position        ,
        E_type_cmd_goto_position        ,
        E_type_cmd_move_position        ,
        E_type_cmd_check_eof_distance   ,
        E_type_ctrl_if                  = K_type_ctrl & K_type_data + 1,
        E_type_ctrl_while               ,
        E_type_ctrl_do_while            ,
        E_type_ctrl_loop_size           ,
        E_type_ctrl_return              = K_type_ctrl + 1,
        E_type_ctrl_break               ,
        E_type_ctrl_continue            ,
        E_type_var_declare,                    // pas fait
        E_type_var_assign,                    // pas fait

        E_type_field_switch             = K_type_field + K_type_data +  1,
        E_type_field_struct             ,

        E_type_field_enum               ,
        E_type_field_string             ,
        E_type_field_string_nl          ,
        E_type_field_raw                ,
        E_type_field_subproto           ,
        E_type_field_insproto           ,

        E_type_field_struct_inline      ,
        E_type_field_basic              ,
        E_type_none                     = 0,
    };
#endif

    enum E_output_directive
    {
        // Do NOT change values, directly used to change output level.
        E_output_directive_hide = -1,
        E_output_directive_none = 0,
        E_output_directive_display = 1
    };

    E_output_directive           output_directive;
    vector<string>               str_arrays;

    string                       new_expression;  // expression for variable, set, call
    vector<string>               fct_parameters;
    scoped_copyable_ptr<T_struct_definition>     P_sub_struct;      // struct fields or if/while bloc  
//    vector<T_field_type_name>    sub_struct;      // struct fields or if/while bloc
    vector<T_field_type_name>    sub_struct_2;    // else bloc
    scoped_copyable_ptr<T_bitfield_definition>   P_bitfield;

    //
    bool    is_an_array() const          { return  str_arrays.empty() == false; }
    bool    is_a_variable() const        { return  (new_expression.empty() == false) && (type != "set"); }

    //
    const string  & get_var_expression() const { return  new_expression; }
    const string  & get_set_expression() const { return  new_expression; }

    // Because of transform (or ...), the resulting value could change.
    bool    must_force_manage_as_biggest_float() const;
    bool    must_force_manage_as_biggest_int() const;

    // Output directive
    bool    must_hide() const       { return  output_directive == E_output_directive_hide; }
    bool    must_display() const    { return  output_directive == E_output_directive_display; }

    // Field external data.
    int                          wsgd_field_idx;

    
};

struct T_field_type_name;

T_field_type_name *  type_definitions_add_field(T_type_definitions  * P_type_definitions,
                                                T_struct_fields     * P_struct_fields);

//****************************************************************************
// Struct
//****************************************************************************

struct T_struct_definition;

T_struct_definition *  type_definitions_create_struct(T_type_definitions  * P_type_definitions, 
                                                      const char          * new_type_name);

void    type_definitions_struct_set_printf_args(T_type_definitions  * P_type_definitions,
                                                T_struct_definition * P_struct_definition,
                                          const char                * printf_args);

void    type_definitions_struct_valid(T_type_definitions  * P_type_definitions,
                                      T_struct_definition * P_struct_definition);

T_struct_fields *  type_definitions_struct_get_fields(T_type_definitions  * P_type_definitions,
                                                      T_struct_definition * P_struct_definition);


//****************************************************************************
// Bitfield
//****************************************************************************

struct T_bitfield_definition;
struct T_bitstream_definition;


T_bitfield_definition *  type_definitions_create_bitfield(T_type_definitions  * P_type_definitions, 
                                                        const char          * new_type_name);
T_bitstream_definition *  type_definitions_create_bitstream(T_type_definitions  * P_type_definitions, 
                                                        const char          * new_type_name);

void    type_definitions_bitfield_set_byte_size(T_type_definitions     * P_type_definitions,
                                                T_bitfield_definition  * P_bitfield_definition,
                                          const int                      byte_size);
void    type_definitions_bitstream_set_byte_size(T_type_definitions     * P_type_definitions,
                                                T_bitstream_definition  * P_bitstream_definition,
                                          const int                      byte_size);

void    type_definitions_bitfield_valid(T_type_definitions  * P_type_definitions,
                                      T_bitfield_definition * P_bitfield_definition);
void    type_definitions_bitstream_valid(T_type_definitions  * P_type_definitions,
                                      T_bitstream_definition * P_bitstream_definition);

T_struct_fields *  type_definitions_bitfield_get_fields(T_type_definitions  * P_type_definitions,
                                                      T_bitfield_definition * P_bitfield_definition);
T_struct_fields *  type_definitions_bitstream_get_fields(T_type_definitions  * P_type_definitions,
                                                      T_bitstream_definition * P_bitstream_definition);



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

struct T_switch_case_value
{
    C_value                case_value;   // value could be string, int, enum symbolic value
    bool                   is_default_case;
    T_struct_fields        fields;        // def

    T_switch_case_value () : is_default_case (false) {}
};

ostream &  operator<< (ostream              & os,
                 const T_switch_case_value  & rhs);

// An switch_case is an array of switch_case values
typedef vector<T_switch_case_value>                   T_switch_case;

struct T_switch_definition
{
    string               case_type;
    T_switch_case        switch_case;
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
};

ostream &  operator<< (ostream               & os,
                 const T_function_parameter  & rhs);

typedef vector<T_function_parameter>                   T_function_parameters;

struct T_function_definition
{
    string                 return_type;
    T_function_parameters  function_parameters;
    T_struct_fields        fields;
};

ostream &  operator<< (ostream                & os,
                 const T_function_definition  & rhs);

// Function name -> function definition representation
typedef map<string,T_function_definition>    T_map_function_definition;

//****************************************************************************
// T_type_definitions
//****************************************************************************

struct T_type_definitions
{
    enum E_type_kind
    {
        E_type_none,
        E_type_alias,
        E_type_struct,
        E_type_bitfield,
        E_type_enum,
        E_type_switch,
        E_type_function
    };

    typedef map<string,E_type_kind>     T_map_forward_type;
    T_map_forward_type                    map_forward_type;

    T_map_alias_type                      map_alias_type;
    T_map_struct_definition               map_struct_definition;
    T_map_bitfield_definition             map_bitfield_definition;
    T_map_enum_definition_representation  map_enum_definition_representation;
    T_map_switch_definition               map_switch_definition;
    T_map_function_definition             map_function_definition;

    // 
    void             add_alias(const string      & new_type,
                               const string      & already_existent_type,
                               const E_override    must_override = E_override_no);

    //
    void    set_field_type(T_field_type_name_base  & field,
                     const string             & type_param) const;
    void    set_field_name(T_field_type_name_base  & field,
                     const string             & name_param) const;

    void    set_field_type_size_or_parameter(T_field_type_name_base  & field,
                               const string             & parameter_param) const;

    void    set_field_transform_quantum(T_field_type_name_base  & field,
                                  const string             & quantum_param) const;
    void    set_field_transform_offset (T_field_type_name_base  & field,
                                  const string             & offset_param) const;

    void    set_field_constraint_min  (T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    set_field_constraint_max  (T_field_type_name_base  & field,
                                 const string             & val_param) const;
    void    set_field_constraint_equal(T_field_type_name_base  & field,
                                 const string             & val_param) const;

    void    set_field_display           (T_field_type_name_base  & field,
                                   const string             & val_param) const;
    void    set_field_display_expression(T_field_type_name_base  & field,
                                   const string             & val_param) const;

    // 
    const T_enum_definition_representation  * get_P_enum    (const string      & type_name) const;
          bool                                is_an_enum    (const string      & type_name) const;

    const T_struct_definition               * get_P_struct  (const string      & type_name) const;
          T_struct_definition               * get_P_struct  (const string      & type_name);
          bool                                is_a_struct   (const string      & type_name) const;

    const T_bitfield_definition             * get_P_bitfield(const string      & type_name) const;

    const T_switch_definition               & get_switch    (const string      & type_name) const;
    const T_switch_definition               * get_P_switch  (const string      & type_name) const;
          bool                                is_a_switch   (const string      & type_name) const;

    const T_function_definition             & get_function  (const string      & type_name) const;
    const T_function_definition             * get_P_function(const string      & type_name) const;


    // Enum : get integer value from symbolic value (<enum type>::<enum symbolic value>).
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
};

ostream &  operator<< (ostream                          & os,
                 const T_type_definitions::E_type_kind  & rhs);

ostream &  operator<< (ostream             & os,
                 const T_type_definitions  & rhs);



#endif /* T_type_definitions_build_interface_h */
