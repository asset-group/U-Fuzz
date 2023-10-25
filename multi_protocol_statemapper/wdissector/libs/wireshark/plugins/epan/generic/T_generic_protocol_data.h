/*
 * Copyright 2008-2020 Olivier Aveline <wsgd@free.fr>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
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

#ifndef __T_GENERIC_PROTOCOL_DATA_H__
#define __T_GENERIC_PROTOCOL_DATA_H__

//*****************************************************************************
// Includes
//*****************************************************************************

#include "config.h"

#include <glib.h>
#include <epan/packet.h>
#include <epan/expert.h>

#include <string>
#include <vector>
#include <utility>
using namespace std;

#include "byte_interpret_build_types.h"
#include "T_interpret_data.h"
#include "T_generic_protocol_data_base.h"
#include "T_generic_statistics.h"


/******************************************************************************
 * T_generic_protocol_ws_data
 *****************************************************************************/

struct T_generic_protocol_fields_data : public CT_debug_object_counter<T_generic_protocol_fields_data>
{
  vector<gint>                hf_id;
  vector<gint>                ett_id;
  vector<hf_register_info>    hf;         // WARNING_ADDRESSES
  vector<gint *>              ett;        // WARNING_ADDRESSES
};

struct T_generic_protocol_enum_value : public CT_debug_object_counter<T_generic_protocol_enum_value>
{
  vector<value_string>    value_strings;
};

struct T_generic_protocol_enum_values_data : public CT_debug_object_counter<T_generic_protocol_enum_values_data>
{
  vector<T_generic_protocol_enum_value>    enum_values;
};

struct T_generic_protocol_expert_data : public CT_debug_object_counter<T_generic_protocol_expert_data>
{
  expert_field              ei_malformed_comment;
  expert_field              ei_malformed_chat;
  expert_field              ei_malformed_note;
  expert_field              ei_malformed_warn;
  expert_field              ei_malformed_error;
  vector<ei_register_info>  ei;

  T_generic_protocol_expert_data()
  {
    expert_field  ei_initializer = EI_INIT;
    ei_malformed_comment = ei_initializer;
    ei_malformed_chat = ei_initializer;
    ei_malformed_note = ei_initializer;
    ei_malformed_warn = ei_initializer;
    ei_malformed_error = ei_initializer;
  }
};

struct T_generic_protocol_subdissector_data : public CT_debug_object_counter<T_generic_protocol_subdissector_data>
{
  dissector_table_t      dissector_table;
  heur_dissector_list_t  heur_dissector_list;
  gboolean               try_heuristic_first;

  static
  dissector_handle_t     data_handle;

  T_generic_protocol_subdissector_data()
      :dissector_table (nullptr),
       heur_dissector_list (nullptr),
       try_heuristic_first (FALSE)
  { }
};

struct T_generic_protocol_tap_data : public CT_debug_object_counter<T_generic_protocol_tap_data>
{
  int                    proto_tap;
  T_stats                stats;
  bool                   tap_is_needed;    // for statistics
  T_RCP_interpret_data   RCP_last_msg_interpret_data;

  T_generic_protocol_tap_data()
      :proto_tap (-1),
       tap_is_needed(false)
  { }
};

struct T_generic_protocol_saved_interpreted_data : public CT_debug_object_counter<T_generic_protocol_saved_interpreted_data>
{
  long                    packet_number;
  long                    msg_number_inside_packet;
  T_RCP_interpret_data    RCP_interpret_data;

  T_generic_protocol_saved_interpreted_data()
      :packet_number(0),
       msg_number_inside_packet(0)
  { }
};

struct T_generic_protocol_global_data : public CT_debug_object_counter<T_generic_protocol_global_data>
{
  // Contains the initial values of global data
  T_RCP_interpret_data    RCP_initialized_data;

  // Contains all interpret_data of msg
  vector<T_generic_protocol_saved_interpreted_data>  saved_interpreted_datas;

  T_generic_protocol_global_data()
      : RCP_initialized_data(new T_interpret_data())
  {
  }
};

struct T_generic_protocol_ws_data : public CT_debug_object_counter<T_generic_protocol_ws_data>
{
  /* Wireshark ID of the protocol */
  int       proto_generic;

  gint      ( * P_dissect_fct )(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
  gboolean  ( * P_heuristic_fct )(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);

  dissector_handle_t                    dissector_handle;

  T_generic_protocol_fields_data        fields_data;
  T_generic_protocol_enum_values_data   enum_values_data;
  T_generic_protocol_expert_data        expert_data;

  T_generic_protocol_subdissector_data  subdissector_data;

  T_generic_protocol_tap_data           tap_data;

  T_generic_protocol_global_data        global_data;

  T_generic_protocol_ws_data()
      :proto_generic (-1),
       P_dissect_fct (nullptr),
       dissector_handle (nullptr)
  { }
};

//*****************************************************************************
// T_generic_protocol_data
//*****************************************************************************

struct T_generic_protocol_data : public T_generic_protocol_data_base
{
    T_generic_protocol_ws_data    ws_data;
};

/******************************************************************************
 * called by dissect_generic_<proto_idx>
 * called by heuristic_generic_<proto_idx>
 *****************************************************************************/
gint
dissect_generic_proto(const int    proto_idx, tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree);

gboolean
heuristic_generic_proto(const int    proto_idx, tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree);

//*****************************************************************************
// The protocol's data
//*****************************************************************************

T_generic_protocol_data  & get_protocol_data(const int   proto_idx);
T_generic_protocol_data  & new_protocol_data(const int   proto_idx);

T_generic_protocol_data  & get_protocol_data_from_proto_abbrev(const char  * proto_abbrev);

void    set_max_nb_of_protocol_data(const size_t   max_nb);

//*****************************************************************************
// read_file_wsgd
//*****************************************************************************

void    read_file_wsgd (const string                   & wsgd_file_name,
                              T_generic_protocol_data  & protocol_data);


#endif /* __T_GENERIC_PROTOCOL_DATA_H__ */
