/* 
 * Copyright 2020 Olivier Aveline <wsgd@free.fr>
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

#include <string>
#include <vector>
#include <sstream>


//*****************************************************************************
// Stats
//*****************************************************************************

struct T_stats_topic
{
    std::string  topic_name;
    std::string  variable_name;    // variable used for statistics
    int          pivot_id = -1;    // stats_tree_create_pivot
};

struct T_stats_sub_group
{
    std::string                 sub_group_name;   // Sub menu of <group_name>
    std::string                 full_name;        // stats_tree st->cfg->name
    std::string                 node_name = "All messages";   // stats_tree_create_node param
    int                         node_id = -1;                 // stats_tree_create_node result
    std::vector<T_stats_topic>  topics;
};

struct T_stats_group
{
    std::string                     group_name;   // Appears into Statistics menu
    std::vector<T_stats_sub_group>  sub_groups;
};

struct T_stats
{
    std::vector<T_stats_group>  groups;

    T_stats_sub_group & get_sub_group_by_full_name(const std::string  & full_name);
};


//*****************************************************************************
// read_file_wsgd_statistics
// iss should be just after STATISTICS keyword
//*****************************************************************************

void    read_file_wsgd_statistics (std::istringstream   & iss,
                                   T_stats              & stats);
