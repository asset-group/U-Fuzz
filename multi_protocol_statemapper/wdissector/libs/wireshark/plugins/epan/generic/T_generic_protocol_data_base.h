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

#pragma once

//*****************************************************************************
// Includes
//*****************************************************************************

#include "config.h"
#include <epan/packet.h>

#include <string>
#include <vector>
#include <sstream>
using namespace std;

#include "CT_debug_object_counter.h"
#include "byte_interpret_common.h"
#include "T_generic_statistics.h"
#include "T_type_definitions.h"


//*****************************************************************************
// T_generic_protocol_data_base
//*****************************************************************************

struct T_generic_protocol_data_base : public CT_debug_object_counter<T_generic_protocol_data_base>
{
    int       proto_idx;  // identify the proto

    bool      is_proto_usable() const        { return  proto_idx >= 0; }
    void      proto_is_NOT_usable()          { proto_idx = -1; }

    string    wsgd_file_name;

    E_debug_status  DEBUG;

    string    PROTONAME;
    string    PROTOSHORTNAME;
    string    PROTOABBREV;

    struct T_parent
    {
        string                   PARENT_SUBFIELD;
        vector<int>              PARENT_SUBFIELD_VALUES_int;
        vector<string>           PARENT_SUBFIELD_VALUES_str;
        vector<pair<int,int> >   PARENT_SUBFIELD_RANGES_int;
    };
    vector<T_parent>  PARENTS;

    vector<string>    PARENTS_HEURISTIC;
    string            HEURISTIC_FUNCTION;

    vector<string>    ADD_FOR_DECODE_AS_TABLES;

    string            SUBPROTO_SUBFIELD;
    string            SUBPROTO_SUBFIELD_PARAM;
    string            SUBPROTO_SUBFIELD_PARAM_UI;
    string            SUBPROTO_SUBFIELD_TYPE;
    ftenum_t          SUBPROTO_SUBFIELD_TYPE_WS;
    string            SUBPROTO_SUBFIELD_FROM_REAL_1;
    string            SUBPROTO_SUBFIELD_FROM_REAL_2;
    string            SUBPROTO_SUBFIELD_FROM_REAL_3;

    bool              PACKET_CONTAINS_ONLY_1_MSG;
    bool              PACKET_CONTAINS_ONLY_COMPLETE_MSG;
    bool              MANAGE_WIRESHARK_PINFO;

    string            MSG_HEADER_TYPE;
    string            MSG_ID_FIELD_NAME;
    string            MSG_TITLE;
    vector<string>    MSG_SUMMARY_SUBSIDIARY_FIELD_NAMES;
    string            MSG_MAIN_TYPE;
    string            MSG_FROM_MAIN_TYPE;
    string            MSG_TO_MAIN_TYPE;
    string            MSG_TOTAL_LENGTH;
    int               MSG_HEADER_LENGTH;
    int               MSG_TRAILER_LENGTH;

    string            GLOBAL_DATA_TYPE;

    T_type_definitions    type_definitions;

    void    check_config_parameters_initialized() const;

    T_generic_protocol_data_base()
        : proto_idx(-1)
        , DEBUG(E_debug_status_OFF)
        , SUBPROTO_SUBFIELD_TYPE_WS(FT_NONE)
        , PACKET_CONTAINS_ONLY_1_MSG(false)
        , PACKET_CONTAINS_ONLY_COMPLETE_MSG(false)
        , MANAGE_WIRESHARK_PINFO(false)
        , MSG_HEADER_LENGTH(-1)
        , MSG_TRAILER_LENGTH(-1)
    { }
};

//*****************************************************************************
// read_file_wsgd_until_types
// Stop after read "PROTO_TYPE_DEFINITIONS"
//*****************************************************************************

void    read_file_wsgd_until_types(
                              istringstream                 & iss,
                              T_generic_protocol_data_base  & protocol_data,
                              T_stats                       & stats);
