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
#include <epan/expert.h>
#include <epan/proto_data.h>
#include "packet-pdcp-lte.h"

void proto_register_mac_lte_framed(void);

/* Initialize the protocol and registered fields. */
int proto_pdcp_lte_framed = -1;
extern int proto_pdcp_lte;
dissector_handle_t pdcp_lte_handle = NULL;

/* Main dissection function. */
static int dissect_pdcp_lte_framed(tvbuff_t *tvb, packet_info *pinfo,
                                   proto_tree *tree, void *data _U_)
{
    gint offset = 0;
    struct pdcp_lte_info *p_pdcp_lte_info;
    tvbuff_t *pdcp_tvb;
    guint8 tag = 0;
    gboolean seqnumLengthTagPresent = FALSE;

    /* Needs to be at least as long as:
       - fixed header bytes
       - tag for data
       - at least one byte of PDCP PDU payload */
    if (tvb_captured_length_remaining(tvb, offset) < (gint)(3 + 2))
    {
        return FALSE;
    }

    /* If redissecting, use previous info struct (if available) */
    p_pdcp_lte_info = (pdcp_lte_info *)p_get_proto_data(wmem_file_scope(), pinfo, proto_pdcp_lte, 0);
    if (p_pdcp_lte_info == NULL)
    {
        /* Allocate new info struct for this frame */
        p_pdcp_lte_info = wmem_new0(wmem_file_scope(), pdcp_lte_info);

        /* Read fixed fields */
        p_pdcp_lte_info->no_header_pdu = (gboolean)tvb_get_guint8(tvb, offset++);
        p_pdcp_lte_info->plane = (enum pdcp_plane)tvb_get_guint8(tvb, offset++);
        if (p_pdcp_lte_info->plane == SIGNALING_PLANE)
        {
            p_pdcp_lte_info->seqnum_length = PDCP_SN_LENGTH_5_BITS;
        }
        p_pdcp_lte_info->rohc.rohc_compression = (gboolean)tvb_get_guint8(tvb, offset++);

        /* Read optional fields */
        while (tag != PDCP_LTE_PAYLOAD_TAG)
        {
            /* Process next tag */
            tag = tvb_get_guint8(tvb, offset++);
            switch (tag)
            {
            case PDCP_LTE_SEQNUM_LENGTH_TAG:
                p_pdcp_lte_info->seqnum_length = tvb_get_guint8(tvb, offset);
                offset++;
                seqnumLengthTagPresent = TRUE;
                break;
            case PDCP_LTE_DIRECTION_TAG:
                p_pdcp_lte_info->direction = tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_LOG_CHAN_TYPE_TAG:
                p_pdcp_lte_info->channelType = (LogicalChannelType)tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_BCCH_TRANSPORT_TYPE_TAG:
                p_pdcp_lte_info->BCCHTransport = (BCCHTransportType)tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_ROHC_IP_VERSION_TAG:
                /* RoHC IP version field is now 1 byte only; let's skip most significant byte
                       to keep backward compatibility with existing UDP framing protocol */
                p_pdcp_lte_info->rohc.rohc_ip_version = tvb_get_guint8(tvb, offset + 1);
                offset += 2;
                break;
            case PDCP_LTE_ROHC_CID_INC_INFO_TAG:
                p_pdcp_lte_info->rohc.cid_inclusion_info = tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_ROHC_LARGE_CID_PRES_TAG:
                p_pdcp_lte_info->rohc.large_cid_present = tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_ROHC_MODE_TAG:
                p_pdcp_lte_info->rohc.mode = (enum rohc_mode)tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_ROHC_RND_TAG:
                p_pdcp_lte_info->rohc.rnd = tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_ROHC_UDP_CHECKSUM_PRES_TAG:
                p_pdcp_lte_info->rohc.udp_checksum_present = tvb_get_guint8(tvb, offset);
                offset++;
                break;
            case PDCP_LTE_ROHC_PROFILE_TAG:
                p_pdcp_lte_info->rohc.profile = tvb_get_ntohs(tvb, offset);
                offset += 2;
                break;
            case PDCP_LTE_CHANNEL_ID_TAG:
                p_pdcp_lte_info->channelId = tvb_get_ntohs(tvb, offset);
                offset += 2;
                break;
            case PDCP_LTE_UEID_TAG:
                p_pdcp_lte_info->ueid = tvb_get_ntohs(tvb, offset);
                offset += 2;
                break;

            case PDCP_LTE_PAYLOAD_TAG:
                /* Have reached data, so get out of loop */
                p_pdcp_lte_info->pdu_length = tvb_reported_length_remaining(tvb, offset);
                continue;

            default:
                /* It must be a recognised tag */
                wmem_free(wmem_file_scope(), p_pdcp_lte_info);
                return 0;
            }
        }

        if ((p_pdcp_lte_info->plane == USER_PLANE) && (seqnumLengthTagPresent == FALSE))
        {
            /* Conditional field is not present */
            wmem_free(wmem_file_scope(), p_pdcp_lte_info);
            return 0;
        }

        /* Store info in packet */
        p_add_proto_data(wmem_file_scope(), pinfo, proto_pdcp_lte, 0, p_pdcp_lte_info);
    }
    else
    {
        offset = tvb_reported_length(tvb) - p_pdcp_lte_info->pdu_length;
    }

    /**************************************/
    /* OK, now dissect as PDCP LTE        */

    /* Create tvb that starts at actual PDCP PDU */
    pdcp_tvb = tvb_new_subset_remaining(tvb, offset);
    call_dissector_only(pdcp_lte_handle, pdcp_tvb, pinfo, tree, data);
    return tvb_captured_length(tvb);
}

void proto_register_pdcp_lte_framed(void)
{
    /* Register protocol. */
    proto_pdcp_lte_framed = proto_register_protocol("pdcp-lte-framed", "PDCP-LTE-FRAMED", "pdcp-lte-framed");

    /* Allow other dissectors to find this one by name. */
    register_dissector("pdcp-lte-framed", dissect_pdcp_lte_framed, proto_pdcp_lte_framed);

    /* Need to find enabled pdcp-lte dissector */
    pdcp_lte_handle = find_dissector("pdcp-lte");
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
