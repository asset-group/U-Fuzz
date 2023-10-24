/*
** WDISSECTOR PROJECT, 2020
** wdissector
** File description:
** wdissector
*/

#pragma once
#ifndef __WDISSECTOR
#define __WDISSECTOR

#define WD_TYPE_FIELD 0
#define WD_TYPE_GROUP 1
#define WD_TYPE_LAYER 2

// Normal includes
#ifndef GEN_PY_MODULE
#include <inttypes.h>
#ifdef __cplusplus
#include <functional>
#endif
// External libs include
#include <glib.h> // glib 2.0
// Wireshark include
#include <config.h>
#include <epan/epan_dissect.h>
#include <epan/ftypes/ftypes-int.h>
#include <epan/proto.h>
#include <epan/tvbuff-int.h>

#else
// Add appropraite types for python module generation
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint64_t unsigned long
#define int64_t signed long
#define int32_t signed int
#define gboolean unsigned char
#define gchar char
#define guint8 unsigned char
#define guint32 unsigned int
#define guchar unsigned char
#define guint unsigned int
#define guint64 unsigned long
#define field_info void
#define header_field_info void
#define gpointer void

/* field types */
enum ftenum {
    FT_NONE, /* used for text labels with no value */
    FT_PROTOCOL,
    FT_BOOLEAN, /* TRUE and FALSE come from <glib.h> */
    FT_CHAR,    /* 1-octet character as 0-255 */
    FT_UINT8,
    FT_UINT16,
    FT_UINT24, /* really a UINT32, but displayed as 6 hex-digits if FD_HEX*/
    FT_UINT32,
    FT_UINT40, /* really a UINT64, but displayed as 10 hex-digits if FD_HEX*/
    FT_UINT48, /* really a UINT64, but displayed as 12 hex-digits if FD_HEX*/
    FT_UINT56, /* really a UINT64, but displayed as 14 hex-digits if FD_HEX*/
    FT_UINT64,
    FT_INT8,
    FT_INT16,
    FT_INT24, /* same as for UINT24 */
    FT_INT32,
    FT_INT40, /* same as for UINT40 */
    FT_INT48, /* same as for UINT48 */
    FT_INT56, /* same as for UINT56 */
    FT_INT64,
    FT_IEEE_11073_SFLOAT,
    FT_IEEE_11073_FLOAT,
    FT_FLOAT,
    FT_DOUBLE,
    FT_ABSOLUTE_TIME,
    FT_RELATIVE_TIME,
    FT_STRING,
    FT_STRINGZ,     /* for use with proto_tree_add_item() */
    FT_UINT_STRING, /* for use with proto_tree_add_item() */
    FT_ETHER,
    FT_BYTES,
    FT_UINT_BYTES,
    FT_IPv4,
    FT_IPv6,
    FT_IPXNET,
    FT_FRAMENUM, /* a UINT32, but if selected lets you go to frame with that number */
    FT_PCRE,     /* a compiled Perl-Compatible Regular Expression object */
    FT_GUID,     /* GUID, UUID */
    FT_OID,      /* OBJECT IDENTIFIER */
    FT_EUI64,
    FT_AX25,
    FT_VINES,
    FT_REL_OID, /* RELATIVE-OID */
    FT_SYSTEM_ID,
    FT_STRINGZPAD, /* for use with proto_tree_add_item() */
    FT_FCWWN,
    FT_NUM_TYPES /* last item number plus one */
};

typedef struct _GPtrArray GPtrArray;
typedef struct _GByteArray GByteArray;
struct _GPtrArray {
    gpointer *pdata;
    guint len;
};

struct _GByteArray {
    guint8 *data;
    guint len;
};

typedef struct _proto_node {
    struct _proto_node *first_child;
    struct _proto_node *last_child;
    struct _proto_node *next;
    struct _proto_node *parent;
    void *finfo;
    void *tree_data;
} proto_node;

/** A protocol tree element. */
typedef proto_node proto_tree;

#endif

#ifdef __cplusplus
#define WD_PUBLIC extern "C"
#else
#define WD_PUBLIC extern
#endif

// Test packets (use offset 49 to dissect correctly with mac-lte-framed dissector)
// Demo packets
WD_PUBLIC uint8_t DEMO_PKT_RRC_CONNECTION_SETUP[128];

WD_PUBLIC uint8_t DEMO_PKT_RRC_SETUP_COMPLETE[122];

WD_PUBLIC uint8_t DEMO_PKT_RRC_RECONFIGURATION[114];

WD_PUBLIC uint8_t DEMO_PKT_NAS_ATTACH_REQUEST[118];

#ifdef __cplusplus
extern "C" {
#endif
// Public API prototypes
// --- Initialization ---
uint8_t wdissector_init(const char *protocol_name);       // Initialize library
void wdissector_set_log_level(enum ws_log_level level);   // Set log level
void wdissector_enable_fast_full_dissection(uint8_t val); // Enable dissection of all fields without string conversion
void wdissector_enable_full_dissection(uint8_t val);
// --- Common dissection functions ---
gboolean packet_set_protocol(const char *lt_arg);
gboolean packet_set_protocol_fast(const char *proto_name);
void packet_dissect(unsigned char *raw_packet, uint32_t packet_length);
void packet_set_direction(int dir);
void packet_cleanup();
void packet_navigate(uint32_t skip_layers, uint32_t skip_groups, uint8_t (*callback)(proto_tree *, uint8_t, uint8_t *)); // Iterates every field of the packet
epan_dissect_t *wdissector_get_edt();

// --- Wireshark version info ---
const char *wdissector_version_info();
const char *wdissector_profile_info();

// --- Filtering related functions ---
// Condition (internal index)
gboolean packet_has_condition(const char *filter);
gboolean packet_register_condition(const char *filter, uint16_t condition_index); // Register rule
void packet_set_condition(uint16_t condition_index);                              // Set rule
gboolean packet_read_condition(uint16_t condition_index);                         // Read rule (true or false)
// Raw Filter array
const char *packet_register_filter(const char *filter);
void packet_set_filter(const char *filter);
gboolean packet_read_filter(const char *filter);

// --- Fields related functions (string) ---
header_field_info *packet_get_header_info(const char *field_name);
header_field_info *packet_register_set_field_hfinfo(const char *field_name); // Register and set field
int packet_get_field_exists(const char *field_name);
field_info *packet_get_field(const char *field_name);
const char *packet_get_field_name(const char *field_name);   // Return field name
const char *packet_get_field_string(const char *field_name); // Return fiels value string
uint32_t packet_get_field_offset(const char *field_name);    // Read field offset
uint32_t packet_get_field_size(const char *field_name);      // Read field size
uint64_t packet_get_field_bitmask(const char *field_name);   // Read field mask (for bitfields)
uint32_t packet_get_field_encoding(const char *field_name);  // Read field encoding (little or big endian)
int packet_get_field_type(const char *field_name);           // Read field type (wireshark equivalent type)
const char *packet_get_field_type_name(const char *field_name);
const char *packet_get_field_encoding_name(const char *field_name);
uint32_t packet_get_field_uint32(const char *field_name);

// --- Fields related functions (field_info - faster) ---

// GET field by header info (header_field_info)
void packet_set_field_hfinfo(header_field_info *hfi);         // Set field (must be called before dissection)
void packet_set_field_hfinfo_all(header_field_info *hfi);     // Set all fields with same name field (must be called before dissection)
int packet_read_field_exists_hfinfo(header_field_info *hfi);  // Check if field with the same hfi exists in packet
field_info *packet_read_field_hfinfo(header_field_info *hfi); // Read field information structure
GPtrArray *packet_read_fields_hfinfo(header_field_info *hfi); // Read multiple fields information structure
// GET field by internal index (hfi_index)
gboolean packet_register_field(const char *field_name, uint16_t field_hfi_index); // Register target field
void packet_register_set_field(const char *field_name, uint16_t field_hfi_index); // Register and set field (simplify if caching is not needed)
void packet_set_field(uint16_t hfi_index);                                        // Set field (must be called before dissection)
field_info *packet_read_field(uint16_t hfi_index);                                // Read field information structure
GPtrArray *packet_read_fields(uint16_t hfi_index);                                // Read multiple fields information structure
// Common Fields functions
field_info *packet_read_field_at(GPtrArray *fields, uint16_t idx); // Read field from field information array
const char *packet_read_field_name(field_info *field_match);       // Return field name
const char *packet_read_field_abbrev(field_info *field_match);     // Return field abbreviation
uint16_t packet_read_field_offset(field_info *field_match);        // Read field offset
uint32_t packet_read_field_size(field_info *field_match);          // Read field size in aligned bytes
uint8_t packet_read_field_size_bits(uint64_t bitmask);             // Read field size in bits
unsigned long packet_read_field_bitmask(field_info *field_match);  // Read field mask (for bitfields)
uint8_t packet_read_field_bitmask_offset(uint64_t bitmask);        // Read field mask bit offset
uint8_t packet_read_field_bitmask_offset_msb(uint64_t bitmask);    // Read field mask from LSB
uint32_t packet_read_field_encoding(field_info *field_match);      // Read field encoding (little or big endian)
int packet_read_field_type(field_info *field_match);               // Read field type (wireshark equivalent type)
const char *packet_read_field_type_name(field_info *field_match);
const char *packet_read_field_encoding_name(field_info *field_match);
const char *packet_read_field_string(field_info *field_match);
unsigned char *packet_read_field_ustring(field_info *field_match);
GByteArray *packet_read_field_bytes(field_info *field_match);
uint32_t packet_read_field_uint32(field_info *field_match);
int32_t packet_read_field_int32(field_info *field_match);
uint64_t packet_read_field_uint64(field_info *field_match);
int64_t packet_read_field_int64(field_info *field_match);
const char *packet_read_value_to_string(uint32_t value, const header_field_info *hfi); // Get packet field string value from header info

// --- Summary related functions ---
const char *packet_show();
const char *packet_summary();
const char *packet_layers();
const char *packet_layer(uint8_t layer_index);
uint32_t packet_layers_count();
const char *packet_relevant_fields();
char *packet_description();
uint8_t packet_direction();
const char *packet_protocol();
const char *packet_show_pdml();
uint8_t packet_field_summary(uint8_t *raw_packet, uint32_t packet_length, const char *field_name);

// --- Misc ---
void wd_log_g(const char *msg);
void wd_log_y(const char *msg);
void wd_log_r(const char *msg);
void set_wd_log_g(void (*wd_func)(const char *));
void set_wd_log_y(void (*wd_func)(const char *));
void set_wd_log_r(void (*wd_func)(const char *));

#ifdef __cplusplus
}
#endif

#endif