/* Routines for MAC LTE format files with context info as header.
 *
 * Martin Mathieson
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
#include <epan/expert.h>
#include "packet-mac-nr.h"
#include <stdio.h>

void proto_register_mac_nr_framed(void);

/* Initialize the protocol and registered fields. */
extern int proto_mac_nr;
static int proto_mac_nr_framed = -1;
extern int ett_mac_nr;
extern expert_field ei_mac_nr_unknown_udp_framing_tag;

/* Subdissectors */
static dissector_handle_t mac_nr_handle;

/* Main dissection function. */
static int dissect_mac_nr_framed(tvbuff_t *tvb, packet_info *pinfo,
                                   proto_tree *tree, void* data _U_)
{
    gint        offset = 0;
    mac_nr_info *p_mac_nr_info;
    tvbuff_t    *mac_tvb;
    guint8      tag;

    /* Do this again on re-dissection to re-discover offset of actual PDU */

       /* Needs to be at least as long as:
       - fixed header bytes
       - tag for data
       - at least one byte of MAC PDU payload */

    if ((size_t)tvb_reported_length_remaining(tvb, offset) < (3+2)) {
        return 5;
    }

   /* If redissecting, use previous info struct (if available) */
    p_mac_nr_info = (mac_nr_info *)p_get_proto_data(wmem_file_scope(), pinfo, proto_mac_nr, 0);
    if (p_mac_nr_info == NULL) {
        /* Allocate new info struct for this frame */
        p_mac_nr_info = wmem_new0(wmem_file_scope(), mac_nr_info);

        /* Read fixed fields */
        p_mac_nr_info->radioType = tvb_get_guint8(tvb, offset++);
        p_mac_nr_info->direction = tvb_get_guint8(tvb, offset++);
        p_mac_nr_info->rntiType = tvb_get_guint8(tvb, offset++);

        /* Read optional fields */
        do {
            /* Process next tag */
            tag = tvb_get_guint8(tvb, offset++);
            switch (tag) {
                case MAC_NR_RNTI_TAG:
                    p_mac_nr_info->rnti = tvb_get_ntohs(tvb, offset);
                    offset += 2;
                    break;
                case MAC_NR_UEID_TAG:
                    p_mac_nr_info->ueid = tvb_get_ntohs(tvb, offset);
                    offset += 2;
                    break;
                case MAC_NR_HARQID:
                    p_mac_nr_info->harqid = tvb_get_guint8(tvb, offset);
                    offset++;
                    break;
                case MAC_NR_FRAME_SUBFRAME_TAG:
                    /* deprecated */
                    offset += 2;
                    break;
                case MAC_NR_PHR_TYPE2_OTHERCELL_TAG:
                    p_mac_nr_info->phr_type2_othercell = tvb_get_guint8(tvb, offset);
                    offset++;
                    break;
                case MAC_NR_FRAME_SLOT_TAG:
                    p_mac_nr_info->sfnSlotInfoPresent = TRUE;
                    p_mac_nr_info->sysframeNumber = tvb_get_ntohs(tvb, offset);
                    p_mac_nr_info->slotNumber = tvb_get_ntohs(tvb, offset+2);
                    offset += 4;
                    break;
                case MAC_NR_PAYLOAD_TAG:
                    /* Have reached data, so set payload length and get out of loop */
                    /* TODO: this is not correct if there is padding which isn't in frame */
                    p_mac_nr_info->length = tvb_reported_length_remaining(tvb, offset);
                    continue;
                default:
                    /* It must be a recognised tag */
                    {
                        proto_item *ti;
                        proto_tree *subtree;

                        col_set_str(pinfo->cinfo, COL_PROTOCOL, "MAC-NR");
                        col_clear(pinfo->cinfo, COL_INFO);
                        ti = proto_tree_add_item(tree, proto_mac_nr, tvb, offset, tvb_reported_length(tvb), ENC_NA);
                        subtree = proto_item_add_subtree(ti, ett_mac_nr);
                        proto_tree_add_expert(subtree, pinfo, &ei_mac_nr_unknown_udp_framing_tag,
                                              tvb, offset-1, 1);
                    }
                    wmem_free(wmem_file_scope(), p_mac_nr_info);
                    return TRUE;
            }
        } while (tag != MAC_NR_PAYLOAD_TAG);

        p_add_proto_data(wmem_file_scope(), pinfo, proto_mac_nr, 0, p_mac_nr_info);
    }
    else {
        offset = tvb_reported_length(tvb) - p_mac_nr_info->length;
    }

    /**************************************/
    /* OK, now dissect as MAC NR          */

    /* Create tvb that starts at actual MAC PDU */
    mac_tvb = tvb_new_subset_remaining(tvb, offset);
    // dissect_mac_nr(mac_tvb, pinfo, tree, NULL);
    call_dissector_only(mac_nr_handle, mac_tvb, pinfo, tree, NULL);

    return TRUE;
}

void proto_register_mac_nr_framed(void)
{
    /* Register protocol. */
    proto_mac_nr_framed = proto_register_protocol("mac-nr-framed", "MAC-NR-FRAMED", "mac-nr-framed");

    /* Allow other dissectors to find this one by name. */
    register_dissector("mac-nr-framed", dissect_mac_nr_framed, proto_mac_nr_framed);
}

void proto_reg_handoff_mac_nr_framed(void){
    mac_nr_handle = find_dissector("mac-nr");
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
