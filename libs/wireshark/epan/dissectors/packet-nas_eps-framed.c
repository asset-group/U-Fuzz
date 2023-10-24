/* Routines for NAS EPS format files with raw PDU.
 *
 * Matheus Eduardo Garbelini
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <epan/packet.h>
#include <epan/proto_data.h>

void proto_register_mac_lte_framed(void);

/* Initialize the protocol and registered fields. */
static int proto_nas_eps_framed = -1;
static  dissector_handle_t nas_eps_handle = NULL;

extern int proto_nas_eps;

/* Main dissection function. */
static int dissect_nas_eps_framed(tvbuff_t *tvb, packet_info *pinfo,
                                   proto_tree *tree, void* data _U_)
{
    gint                 offset = 0;
    
    if (!nas_eps_handle) {
        return 0;
    }

    /* Do this again on re-dissection to re-discover offset of actual PDU */

    /* Needs to be at least as long as:
       - at least one byte of MAC PDU payload */
    if ((size_t)tvb_reported_length_remaining(tvb, offset) < (1)) {
        return 0;
    }

    /**************************************/
    /* OK, now dissect as NAS EPS         */
    call_dissector_only(nas_eps_handle, tvb, pinfo, tree, NULL);
    return tvb_captured_length(tvb);
}

void proto_register_nas_eps_framed(void)
{
    /* Register protocol. */
    proto_nas_eps_framed = proto_register_protocol("nas-eps-framed", "NAS-EPS-FRAMED", "nas-eps-framed");
    /* Create tvb that starts at actual MAC PDU */
    //  nas_eps_tvb = tvb_new_subset_remaining(tvb, offset);
    /* Allow other dissectors to find this one by name. */
    register_dissector("nas-eps-framed", dissect_nas_eps_framed, proto_nas_eps_framed);
}

void
proto_reg_handoff_nas_eps_framed(void)
{
	 /* Need to find enabled nas-eps dissector */
    nas_eps_handle = find_dissector("nas-eps");
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
