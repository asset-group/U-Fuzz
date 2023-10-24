#pragma once
#ifndef SHM_NR_UTILS_
#define SHM_NR_UTILS_

#include "common/utils/T/shared_memory.h"

#define SHM_MUTEX_0 0
#define SHM_MUTEX_1 1

#define MAC_NR_PAYLOAD_TAG    0x01
#define MAC_NR_RNTI_TAG       0x02
#define MAC_NR_UEID_TAG       0x03
#define MAC_NR_FRAME_SLOT_TAG 0x07

#define NR_FDD_RADIO 1
#define NR_TDD_RADIO 2

#define NR_DIRECTION_UPLINK   0
#define NR_DIRECTION_DOWNLINK 1

#define NR_NO_RNTI 0
#define NR_RA_RNTI 2
#define NR_C_RNTI  3
#define NR_SI_RNTI 4


#define W_GNB_PHY_MIB         (200)
#define W_GNB_MAC_UE_DL_SIB   (201)
#define W_GNB_PHY_INITIATE_RA_PROCEDURE (202)
#define W_GNB_MAC_UE_DL_RAR_PDU_WITH_DATA (203)
#define W_GNB_MAC_UE_DL_PDU_WITH_DATA (204)
#define W_GNB_MAC_UE_UL_PDU_WITH_DATA (205)
#define W_GNB_PDCP_PLAIN (206)
#define W_GNB_PDCP_ENC (207)

// --------------------------------- PDCP --------------------------------------

/* Conditional field. This field is mandatory in case of User Plane PDCP PDU.
   The format is to have the tag, followed by the value (there is no length field,
   it's implicit from the tag). The allowed values are defined above. */

#define PDCP_NR_SEQNUM_LENGTH_TAG          0x02
/* 1 byte */

/* Optional fields. Attaching this info should be added if available.
   The format is to have the tag, followed by the value (there is no length field,
   it's implicit from the tag) */

#define PDCP_NR_DIRECTION_TAG              0x03
/* 1 byte */

#define PDCP_NR_BEARER_TYPE_TAG            0x04
/* 1 byte */

#define PDCP_NR_BEARER_ID_TAG              0x05
/* 1 byte */

#define PDCP_NR_UEID_TAG                   0x06
/* 2 bytes, network order */

#define PDCP_NR_ROHC_COMPRESSION_TAG       0x07
/* 0 byte */

/* N.B. The following ROHC values only have significance if rohc_compression
   is in use for the current channel */

#define PDCP_NR_ROHC_IP_VERSION_TAG        0x08
/* 1 byte */

#define PDCP_NR_ROHC_CID_INC_INFO_TAG      0x09
/* 0 byte */

#define PDCP_NR_ROHC_LARGE_CID_PRES_TAG    0x0A
/* 0 byte */

#define PDCP_NR_ROHC_MODE_TAG              0x0B
/* 1 byte */

#define PDCP_NR_ROHC_RND_TAG               0x0C
/* 0 byte */

#define PDCP_NR_ROHC_UDP_CHECKSUM_PRES_TAG 0x0D
/* 0 byte */

#define PDCP_NR_ROHC_PROFILE_TAG           0x0E
/* 2 bytes, network order */

#define PDCP_NR_MACI_PRES_TAG              0x0F
/* 0 byte */

#define PDCP_NR_SDAP_HEADER_TAG            0x10
/* 1 byte, bitmask with PDCP_NR_UL_SDAP_HEADER_PRESENT and/or PDCP_NR_DL_SDAP_HEADER_PRESENT */

#define PDCP_NR_CIPHER_DISABLED_TAG        0x11
/* 0 byte */

/* PDCP PDU. Following this tag comes the actual PDCP PDU (there is no length, the PDU
   continues until the end of the frame) */
#define PDCP_NR_PAYLOAD_TAG                0x01



inline uint16_t send_pdu_data_nr(int event,
                                 int direction,
                                 int rnti_type,
                                 int rnti,
                                 int frame,
                                 int slot,
                                 uint8_t *payload, int payload_size,
                                 int8_t preamble)
{
   uint16_t payload_offset;
   uint16_t s_idx = 2;
   uint8_t *shm_buffer_ptr = local_sync.shared_memory[SHM_MUTEX_0];

   shm_buffer_ptr[s_idx++] = event;
   // Wireshark payload starts here
   shm_buffer_ptr[s_idx++] = NR_TDD_RADIO;
   shm_buffer_ptr[s_idx++] = direction;
   shm_buffer_ptr[s_idx++] = rnti_type;
   if (rnti_type == NR_C_RNTI || rnti_type == NR_RA_RNTI)
   {
      shm_buffer_ptr[s_idx++] = MAC_NR_RNTI_TAG;
      shm_buffer_ptr[s_idx++] = (rnti >> 8) & 0xFF;
      shm_buffer_ptr[s_idx++] = rnti & 0xFF;
   }

   shm_buffer_ptr[s_idx++] = MAC_NR_FRAME_SLOT_TAG;
   shm_buffer_ptr[s_idx++] = (frame>>8) & 0xFF;
   shm_buffer_ptr[s_idx++] = frame & 0xFF;
   shm_buffer_ptr[s_idx++] = (slot>>8) & 0xFF;
   shm_buffer_ptr[s_idx++] = slot & 0xFF;

   shm_buffer_ptr[s_idx++] = MAC_NR_PAYLOAD_TAG;
   payload_offset = s_idx;
   memcpy(shm_buffer_ptr + s_idx,
          payload,
          payload_size);
   s_idx += payload_size;
   // Place length on first index
   shm_buffer_ptr[0] = s_idx & 0xFF;
   shm_buffer_ptr[1] = (s_idx >> 8) & 0xFF;
   // Notify new payload
   shm_notify(SHM_MUTEX_0);
   // Wait response
   if(direction == NR_DIRECTION_DOWNLINK)
   {
      shm_wait(SHM_MUTEX_0);
      memcpy(payload, local_sync.shared_memory[SHM_MUTEX_0] + payload_offset, payload_size);
   }

   return payload_offset; // Return offset where payload was inserted in shared memory
}

static inline uint16_t send_pdu_data_pdcp_nr(int direction,
                                          int plane_type,
                                          int bearer_id,
                                          int has_integrity,
                                          int rohc_profile,
                                          int seqnum_length,
                                          int ueid,
                                          uint8_t *payload, int payload_size)
{
   uint16_t payload_offset;
   uint16_t s_idx = 2;
   uint8_t *shm_buffer_ptr = local_sync.shared_memory[SHM_MUTEX_1];

   shm_buffer_ptr[s_idx++] = (has_integrity > 0 ? W_GNB_PDCP_ENC : W_GNB_PDCP_PLAIN); // Event
   // Wireshark payload starts here
   shm_buffer_ptr[s_idx++] = FALSE;                      // No header PDU (True only for NB-IoT)
   shm_buffer_ptr[s_idx++] = plane_type;                 // Plane type (User plane or Control plane)
   shm_buffer_ptr[s_idx++] = (rohc_profile > 0 ? 1 : 0); // Has rohc compression
   // Dynamic fields
   shm_buffer_ptr[s_idx++] = PDCP_NR_DIRECTION_TAG;
   shm_buffer_ptr[s_idx++] = direction;
   shm_buffer_ptr[s_idx++] = PDCP_NR_BEARER_TYPE_TAG;
   shm_buffer_ptr[s_idx++] = 1; //DCCH
   shm_buffer_ptr[s_idx++] = PDCP_NR_SEQNUM_LENGTH_TAG;
   shm_buffer_ptr[s_idx++] = seqnum_length;

   shm_buffer_ptr[s_idx++] = PDCP_NR_BEARER_ID_TAG;
   shm_buffer_ptr[s_idx++] = (bearer_id >> 8) & 0xFF;
   shm_buffer_ptr[s_idx++] = bearer_id & 0xFF;

   shm_buffer_ptr[s_idx++] = PDCP_NR_UEID_TAG;
   shm_buffer_ptr[s_idx++] = (ueid >> 8) & 0xFF;
   shm_buffer_ptr[s_idx++] = ueid & 0xFF;

   shm_buffer_ptr[s_idx++] = PDCP_NR_ROHC_PROFILE_TAG;
   shm_buffer_ptr[s_idx++] = (rohc_profile >> 8) & 0xFF;
   shm_buffer_ptr[s_idx++] = rohc_profile & 0xFF;

   shm_buffer_ptr[s_idx++] = PDCP_NR_PAYLOAD_TAG;
   payload_offset = s_idx;
   memcpy(shm_buffer_ptr + s_idx,
          payload,
          payload_size);
   s_idx += payload_size;
   // Place length on first index
   shm_buffer_ptr[0] = s_idx & 0xFF;
   shm_buffer_ptr[1] = (s_idx >> 8) & 0xFF;
   // Notify new payload
   shm_notify(SHM_MUTEX_1);
   // Wait response
   if(direction == NR_DIRECTION_DOWNLINK) {
      shm_wait(SHM_MUTEX_1);
      memcpy(payload, local_sync.shared_memory[SHM_MUTEX_1] + payload_offset, payload_size);
   }

   return payload_offset; // Return offset where payload was inserted in shared memory
}


#endif

