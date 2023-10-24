// Normal includes
#include <config.h>
#include <errno.h>
#include <functional>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
// #pragma intrinsic
#include <x86intrin.h>

// External libs include
#include <glib.h> // glib 2.0
// #include <glibconfig.h>

// Wireshark includes
#include "capture/capture-pcap-util.h"
#include "epan/column-utils.h"
#include "epan/proto.h"
#include "extcap.h"
#include "file.h"
#include "frame_tvbuff.h"
#include "globals.h"
#include "ui/capture_ui_utils.h"
#include "ui/dissect_opts.h"
#include "ui/failure_message.h"
#include "ui/util.h"
#include <cli_main.h>
#include <epan/addr_resolv.h>
#include <epan/column.h>
#include <epan/disabled_protos.h>
#include <epan/epan.h>
#include <epan/epan_dissect.h>
#include <epan/ftypes/ftypes-int.h>
#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/print.h>
#include <epan/proto.h>
#include <epan/stat_tap_ui.h>
#include <epan/tap.h>
#include <epan/timestamp.h>
#include <epan/tvbuff-int.h>
#include <epan/uat-int.h>
#include <epan/uat.h>
#include <setjmp.h>
#include <ui/clopts_common.h>
#include <ui/cmdarg_err.h>
#include <ui/version_info.h>
#include <wiretap/libpcap.h>
#include <wiretap/pcap-encap.h>
#include <wiretap/wtap.h>
#include <wiretap/wtap_opttypes.h>
#include <wsutil/file_util.h>
#include <wsutil/filesystem.h>
#include <wsutil/plugins.h>
#include <wsutil/privileges.h>
#include <wsutil/report_message.h>
#include <wsutil/wslog.h>

// Project includes
#include "libs/logs.h"
#include "libs/profiling.h"
#include "libs/tinydir.h"
#include "libs/whereami.h"
#include "wdissector.h"
#include <config.h>

#define DEFAULT_PROFILE_NAME "WDissector"
#define DEFAULT_LINK_TYPE "encap:1"

uint8_t DEMO_PKT_RRC_CONNECTION_SETUP[128] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    0x56, 0xde, 0x35, 0x40, 0x0, 0x40, 0x11, 0x5e,
    0x5f, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    0x1, 0xc9, 0xb6, 0x27, 0xf, 0x0, 0x42, 0xfe,
    0x55, 0x6d, 0x61, 0x63, 0x2d, 0x6c, 0x74, 0x65,
    0x1, 0x1, 0x3, 0x2, 0x28, 0xc3, 0x4, 0xc,
    0x96, 0x1, 0x3c, 0x20, 0x1d, 0x1f, 0x41, 0xa1,
    0xeb, 0x6e, 0x2c, 0x88, 0x68, 0x12, 0x98, 0xf,
    0x1c, 0xce, 0x1, 0x83, 0x80, 0xba, 0x30, 0x79,
    0x43, 0xfb, 0x80, 0x4, 0x23, 0x80, 0x89, 0x1a,
    0x2, 0x44, 0x68, 0xd, 0x90, 0x10, 0x8e, 0xa,
    0x0, 0x0, 0x0};

uint8_t DEMO_PKT_RRC_SETUP_COMPLETE[122] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    0x6c, 0xde, 0x36, 0x40, 0x0, 0x40, 0x11, 0x5e,
    0x48, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    0x1, 0xc9, 0xb6, 0x27, 0xf, 0x0, 0x58, 0xfe,
    0x6b, 0x6d, 0x61, 0x63, 0x2d, 0x6c, 0x74, 0x65,
    0x1, 0x0, 0x3, 0x2, 0x28, 0xc3, 0x4, 0xc,
    0xb0, 0x1, 0x3a, 0x3d, 0x21, 0xe, 0x1f, 0x9,
    0x0, 0xa0, 0x0, 0x0, 0x22, 0x0, 0x9, 0x8e,
    0x10, 0xff, 0xd2, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0};

uint8_t DEMO_PKT_RRC_RECONFIGURATION[114] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    0x64, 0xde, 0x42, 0x40, 0x0, 0x40, 0x11, 0x5e,
    0x44, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    0x1, 0xc9, 0xb6, 0x27, 0xf, 0x0, 0x50, 0xfe,
    0x63, 0x6d, 0x61, 0x63, 0x2d, 0x6c, 0x74, 0x65,
    0x1, 0x1, 0x3, 0x2, 0x28, 0xc3, 0x4, 0xd,
    0x12, 0x1, 0x21, 0x30, 0x1f, 0xa0, 0x3, 0x2,
    0x22, 0x2, 0x35, 0x38, 0x4, 0x67, 0x9c, 0x23,
    0x27, 0x0, 0x3e, 0xa0, 0x5f, 0x1c, 0xe1, 0xd8,
    0x85, 0xba, 0x30, 0x7d, 0xa8, 0x78, 0x0, 0x18,
    0x7, 0xf7, 0x0, 0x8, 0x47, 0x1, 0x12, 0x34,
    0x4, 0x88, 0xd0, 0x1b, 0x20, 0x21, 0x1c, 0x14,
    0x0, 0x92, 0x86, 0x3b, 0xa4, 0x0, 0x0, 0x0,
    0x0};

uint8_t DEMO_PKT_NAS_ATTACH_REQUEST[118] = {
    0x17, 0x37, 0x7b, 0xf3, 0x9f, 0x22, 0x7, 0x41, 0x2,
    0xb, 0xf6, 0x9, 0xf1, 0x7, 0x0, 0x2, 0x1,
    0xe5, 0x0, 0x86, 0xf3, 0x5, 0xf0, 0x70, 0xc0,
    0x40, 0x19, 0x0, 0x2a, 0x2, 0x10, 0xd0, 0x11,
    0xd1, 0x27, 0x23, 0x80, 0x80, 0x21, 0x10, 0x1,
    0x0, 0x0, 0x10, 0x81, 0x6, 0x0, 0x0, 0x0,
    0x0, 0x83, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xd, 0x0, 0x0, 0xa, 0x0, 0x0, 0x5, 0x0,
    0x0, 0x10, 0x0, 0x0, 0x11, 0x0, 0x52, 0x9,
    0xf1, 0x7, 0x0, 0x1, 0x5c, 0xa, 0x0, 0x31,
    0x3, 0xe5, 0xe0, 0x3e, 0x90, 0x11, 0x3, 0x57,
    0x58, 0xa6, 0x20, 0xa, 0x60, 0x14, 0x4, 0x22,
    0x91, 0x81, 0xf, 0x1a, 0x1e, 0x50, 0x40, 0x8,
    0x4, 0x2, 0x60, 0x4, 0x0, 0x2, 0x1f, 0x2,
    0x5d, 0x1, 0x2, 0xe0, 0xc1};

// #define NODE_NOT_EXPERT_INFO(node) node->finfo->hfinfo != expert_info;

static void (*wd_log_fcn_g)(const char *msg) = NULL;
static void (*wd_log_fcn_y)(const char *msg) = NULL;
static void (*wd_log_fcn_r)(const char *msg) = NULL;

// Dissection Instance variables
static int encap;
static uint8_t epan_initialized = 0;
static gint (*field_callback)(field_info *fi) = NULL;
static e_prefs *prefs_p;
capture_file cfile;  // Capture File declaration
static wtap_rec rec; // record
static epan_dissect_t edt;
static int packet_dir = PACK_FLAGS_DIRECTION_UNKNOWN;
wtap_block_t g_wtap_block;
uint32_t *g_wtap_packet_direction;

// Enable or disable full dissection of the tree
static uint8_t full_dissection = FALSE;
static uint8_t fast_full_dissection = FALSE;

// DFilter vards
static dfilter_t *filtercodes[1024]; // Store compiled filters
static uint16_t filtercode_index;
static uint32_t packet_proto_ids[256];
static header_field_info *fields_hfi[2048] = {NULL};
static uint16_t fields_hfi_index;

// Frame vars
static guint32 cum_bytes;
static frame_data ref_frame;
static frame_data prev_dis_frame;
static frame_data prev_cap_frame;

// DLT Vars
uat_t *user_dlt_module_uat = NULL;
extern "C" struct epan_uat *prefs_get_uat_value(pref_t *pref);

// user_dlt struct
typedef struct _user_encap_t {
    guint encap;
    char *payload_proto_name;
    dissector_handle_t payload_proto;
    char *header_proto_name;
    dissector_handle_t header_proto;
    char *trailer_proto_name;
    dissector_handle_t trailer_proto;
    guint header_size;
    guint trailer_size;
} user_encap_t;

// Functions

void wd_log_g(const char *msg)
{
    if (wd_log_fcn_g)
        wd_log_fcn_g(msg);
    else
        puts(msg);
}

void wd_log_y(const char *msg)
{
    if (wd_log_fcn_y)
        wd_log_fcn_y(msg);
    else
        puts(msg);
}

void wd_log_r(const char *msg)
{
    if (wd_log_fcn_r)
        wd_log_fcn_r(msg);
    else
        puts(msg);
}

void set_wd_log_g(void (*wd_func)(const char *))
{
    wd_log_fcn_g = wd_func;
}
void set_wd_log_y(void (*wd_func)(const char *))
{
    wd_log_fcn_y = wd_func;
}
void set_wd_log_r(void (*wd_func)(const char *))
{
    wd_log_fcn_r = wd_func;
}

const char *packet_read_value_to_string(uint32_t value, const header_field_info *hfinfo)
{
    if (hfinfo->display & BASE_RANGE_STRING)
        return try_rval_to_str(value, (const range_string *)hfinfo->strings);

    if (hfinfo->display & BASE_EXT_STRING) {
        if (hfinfo->display & BASE_VAL64_STRING)
            return try_val64_to_str_ext(value, (val64_string_ext *)hfinfo->strings);
        else
            return try_val_to_str_ext(value, (value_string_ext *)hfinfo->strings);
    }

    if (hfinfo->display & BASE_VAL64_STRING)
        return try_val64_to_str(value, (const val64_string *)hfinfo->strings);

    if (hfinfo->display & BASE_UNIT_STRING)
        return unit_name_string_get_value(value, (const struct unit_name_string *)hfinfo->strings);

    return try_val_to_str(value, (const value_string *)hfinfo->strings);
}

const char *
packet_get_value_to_string(guint32 value, const char *field_name)
{
    header_field_info *hfi = packet_get_header_info(field_name);
    if (hfi)
        return packet_read_value_to_string(value, hfi);
    else
        return NULL;
}

// Set protocol fast (do not append "proto:")
gboolean packet_set_protocol_fast(const char *proto_name)
{
    if (user_dlt_module_uat) {
        // Update initialized user dlt table
        dissector_handle_t dhandle = find_dissector(proto_name);
        if (dhandle) {
            encap = WTAP_ENCAP_USER0;
            user_encap_t *rec = (user_encap_t *)UAT_USER_INDEX_PTR(user_dlt_module_uat, 0);
            g_free(rec->payload_proto_name);
            rec->payload_proto_name = g_strdup(proto_name);
            rec->payload_proto = dhandle;
        }
        return TRUE;
    }
    return FALSE;
}

/**
 * Parse a link-type argument of the form "encap:<pcap linktype>" or
 * "proto:<proto name>".  "Pcap linktype" must be a name conforming to
 * pcap_datalink_name_to_val() or an integer; the integer should be
 * a LINKTYPE_ value supported by Wiretap.  "Proto name" must be
 * a protocol name, e.g. "http".
 */
gboolean packet_set_protocol(const char *lt_arg)
{
    const char *spec_ptr = strchr(lt_arg, ':');
    char *p;
    int dlt_val;
    long val;
    dissector_handle_t dhandle;
    GString *pref_str;
    char *errmsg = NULL;

    if (epan_initialized) {
        epan_initialized = 0;
        epan_dissect_cleanup(&edt);
    }

    if (!spec_ptr)
        return FALSE;

    spec_ptr++;
    if (strncmp(lt_arg, "encap:", strlen("encap:")) == 0) {
        dlt_val = linktype_name_to_val(spec_ptr);
        // printf("spec_ptr:%s\n",spec_ptr);
        if (dlt_val == -1) {
            errno = 0;
            val = strtol(spec_ptr, &p, 10);
            if (p == spec_ptr || *p != '\0' || errno != 0 || val > INT_MAX) {
                return FALSE;
            }
            dlt_val = (int)val;
        }
        /*
         * In those cases where a given link-layer header type
         * has different LINKTYPE_ and DLT_ values, linktype_name_to_val()
         * will return the OS's DLT_ value for that link-layer header
         * type, not its OS-independent LINKTYPE_ value.
         *
         * On a given OS, wtap_pcap_encap_to_wtap_encap() should
         * be able to map either LINKTYPE_ values or DLT_ values
         * for the OS to the appropriate Wiretap encapsulation.
         */
        encap = wtap_pcap_encap_to_wtap_encap(dlt_val);
        if (encap == WTAP_ENCAP_UNKNOWN) {
            return FALSE;
        }
        return TRUE;
    }
    else if (strncmp(lt_arg, "proto:", strlen("proto:")) == 0) {
        dhandle = find_dissector(spec_ptr);
        if (dhandle) {
            encap = WTAP_ENCAP_USER0;

            if (user_dlt_module_uat) {
                // Update initialized user dlt table
                user_encap_t *rec = (user_encap_t *)UAT_USER_INDEX_PTR(user_dlt_module_uat, 0);
                g_free(rec->payload_proto_name);
                rec->payload_proto_name = g_strdup(spec_ptr);
                rec->payload_proto = dhandle;
                return TRUE;
            }

            pref_str = g_string_new("uat:user_dlts:");
            /* This must match the format used in the user_dlts file */
            g_string_append_printf(pref_str,
                                   "\"User 0 (DLT=147)\",\"%s\",\"0\",\"\",\"0\",\"\"",
                                   spec_ptr);

            // prefs_reset(); // Reset preferences here so dissection works (Not needed)
            if (prefs_set_pref(pref_str->str, &errmsg) != PREFS_SET_OK) {
                g_string_free(pref_str, TRUE);
                g_free(errmsg);
                return FALSE;
            }
            g_string_free(pref_str, TRUE);
            // Store user dlt prefs for fast update later
            module_t *user_dlt_module = prefs_find_module("user_dlt");
            pref_t *user_dlt_prefs = prefs_find_preference(user_dlt_module, "encaps_table");
            user_dlt_module_uat = prefs_get_uat_value(user_dlt_prefs);
            return TRUE;
        }
    }
    return FALSE;
}

static const nstime_t *
raw_get_frame_ts(struct packet_provider_data *prov, guint32 frame_num)
{
    if (prov->ref && prov->ref->num == frame_num)
        return &prov->ref->abs_ts;

    if (prov->prev_dis && prov->prev_dis->num == frame_num)
        return &prov->prev_dis->abs_ts;

    if (prov->prev_cap && prov->prev_cap->num == frame_num)
        return &prov->prev_cap->abs_ts;

    return NULL;
}

static epan_t *raw_epan_new(capture_file *cf)
{
    static const struct packet_provider_funcs funcs = {
        raw_get_frame_ts,
        cap_file_provider_get_interface_name,
        cap_file_provider_get_interface_description,
        NULL,
    };

    return epan_new(&cf->provider, &funcs);
}

const char *wdissector_version_info()
{
    return get_appname_and_version();
}

const char *wdissector_profile_info()
{
    return DEFAULT_PROFILE_NAME;
}

void wdissector_enable_fast_full_dissection(uint8_t val)
{
    if (epan_initialized)
        edt.tree->tree_data->fast_full_dissection = val;
    else
        fast_full_dissection = val;
}

void wdissector_set_field_callback(gint (*fcn_callback)(field_info *fi))
{
    if (epan_initialized)
        edt.tree->tree_data->field_callback = fcn_callback;
    else
        field_callback = fcn_callback;
}

void wdissector_enable_full_dissection(uint8_t val)
{
    full_dissection = val;
}

epan_dissect_t *wdissector_get_edt()
{
    return &edt;
}

void wdissector_set_log_level(enum ws_log_level level)
{
    ws_log_set_level(level);
}

uint8_t wdissector_init(const char *protocol_name)
{
    uint8_t return_status = 1;
    setlocale(LC_ALL, "");

    // Initialize log API with LOG_LEVEL_NONE
    ws_log_init("wdissector", vcmdarg_err);
    ws_log_set_level(LOG_LEVEL_NONE);

    /* Initialize the version information. */
    ws_init_version_info("WDissector Lib (Wireshark)",
                         epan_gather_compile_info,
                         NULL);

    // printf("%s\n", get_appname_and_version());

    /*
     * Get credential information for later use.
     */
    init_process_policies();

    /*
     * Attempt to get the pathname of the directory containing the
     * executable file.
     */

    char current_path[128];
    wai_getExecutablePath(current_path, sizeof(current_path), NULL);

    char *init_progfile_dir_error = configuration_init(current_path, NULL);
    if (init_progfile_dir_error != NULL) {
        printf("Can't get current pathname: %s\n", current_path);
        return_status = 0;
    }

    if (profile_exists(DEFAULT_PROFILE_NAME, TRUE)) {
        set_profile_name(DEFAULT_PROFILE_NAME);
        // printf("Dissection Profile \"%s\" loaded\n", DEFAULT_PROFILE_NAME);
    }
    else {
        printf("Profile \"%s\" not found!\n", DEFAULT_PROFILE_NAME);
        return_status = 0;
    }

    timestamp_set_type(TS_RELATIVE);
    timestamp_set_precision(TS_PREC_AUTO);
    timestamp_set_seconds_type(TS_SECONDS_DEFAULT);

    wtap_init(TRUE);

    /* Register all dissectors; we must do this before checking for the
       "-G" flag, as the "-G" flag dumps information registered by the
       dissectors, and we must do it before we read the preferences, in
       case any dissectors register preferences. */
    if (!epan_init(NULL, NULL, TRUE)) {
        printf("epan couldn't initialize\n");
        return_status = 0;
        exit(1);
    }

    /* Load libwireshark settings from the current profile. */
    prefs_p = epan_load_settings();

    cap_file_init(&cfile);

    // Disable DNS name resolution
    disable_name_resolution();

    /* Notify all registered modules that have had any of their preferences
       changed either from one of the preferences file or from the command
       line that their preferences have changed.
       Initialize preferences before display filters, otherwise modules
       like MATE won't work. */
    prefs_apply_all();

    /*
     * Enabled and disabled protocols and heuristic dissectors as per
     * command-line options.
     */
    setup_enabled_and_disabled_protocols();

    /* Build the column format array */
    build_column_format_array(&cfile.cinfo, prefs_p->num_cols, TRUE);

    /*
     * Immediately relinquish any special privileges we have; we must not
     * be allowed to read any capture files the user running Rawshark
     * can't open.
     */
    relinquish_special_privs_perm();
    if (protocol_name == NULL || !packet_set_protocol(protocol_name)) {
        packet_set_protocol(DEFAULT_LINK_TYPE);
    }

    /* Make sure we got a dissector handle for our payload. */
    if (encap == WTAP_ENCAP_UNKNOWN) {
        printf("No valid payload dissector specified\n");
        return_status = 0;
        exit(2);
    }

    /* Create new epan session for dissection. */
    epan_free(cfile.epan);
    cfile.epan = raw_epan_new(&cfile);
    cfile.provider.wth = NULL;
    cfile.f_datalen = 0; /* not used, but set it anyway */

    /* Set the file name because we need it to set the follow stream filter.
       XXX - is that still true?  We need it for other reasons, though,
       in any case. */
    cfile.filename = g_strdup("dissection");

    /* Indicate whether it's a permanent or temporary file. */
    cfile.is_tempfile = FALSE;

    /* No user changes yet. */
    cfile.unsaved_changes = FALSE;
    cfile.cd_t = WTAP_FILE_TYPE_SUBTYPE_UNKNOWN;
    cfile.open_type = WTAP_TYPE_AUTO;
    cfile.count = 0;
    cfile.drops_known = FALSE;
    cfile.drops = 0;
    cfile.snap = 0;
    nstime_set_zero(&cfile.elapsed_time);
    cfile.provider.ref = NULL;
    cfile.provider.prev_dis = NULL;
    cfile.provider.prev_cap = NULL;

    wtap_rec_init(&rec);
    rec.rec_type = REC_TYPE_PACKET;
    rec.presence_flags = WTAP_HAS_TS | WTAP_HAS_CAP_LEN;
    rec.ts.secs = 0;
    rec.ts.nsecs = 0;

    // Initialize wtap block for link layer flags (Direction, etc)
    g_wtap_block = wtap_block_create(WTAP_BLOCK_PACKET);
    wtap_block_add_uint32_option(g_wtap_block, OPT_PKT_FLAGS, packet_dir);
    wtap_optval_t *g_wtap_packet_flag = wtap_block_get_option(g_wtap_block, OPT_PKT_FLAGS);
    // Get wtap block pointer of packet flags for faster access
    g_wtap_packet_direction = &g_wtap_packet_flag->uint32val;

    // Initialize filtercodes array with NULL
    memset(filtercodes, 0, sizeof(filtercodes));
    return return_status;
}

void packet_set_direction(int dir)
{
    switch (dir) {
    case P2P_DIR_RECV:
        packet_dir = PACK_FLAGS_DIRECTION_INBOUND;
        break;
    case P2P_DIR_SENT:
        packet_dir = PACK_FLAGS_DIRECTION_OUTBOUND;
        break;

    default:
        packet_dir = PACK_FLAGS_DIRECTION_UNKNOWN;
        break;
    }
}

void packet_dissect(unsigned char *raw_packet, uint32_t packet_length)
{
    frame_data fdata;

    if (!epan_initialized) {
        // Initializes epan with new full_dissection parameter
        epan_initialized = 1;
        epan_dissect_init(&edt, cfile.epan, TRUE, full_dissection);
        edt.tree->tree_data->fast_full_dissection = fast_full_dissection;
        edt.tree->tree_data->field_callback = field_callback;
    }
    else if (edt.pi.data_src) {
        // Reset edt before dissection (saves time)
        epan_dissect_reset(&edt);
    }

    rec.rec_header.packet_header.caplen = packet_length;
    rec.rec_header.packet_header.len = packet_length;
    rec.rec_header.packet_header.pkt_encap = encap;
    // --

    cfile.count++;
    frame_data_init(&fdata, cfile.count, &rec, 0, cum_bytes);

    frame_data_set_before_dissect(&fdata, &cfile.elapsed_time,
                                  &cfile.provider.ref, cfile.provider.prev_dis);
    if (cfile.provider.ref == &fdata) {
        ref_frame = fdata;
        cfile.provider.ref = &ref_frame;
    }

    // Set direction using wtap block
    rec.block = g_wtap_block;              // Recover global rec wtap block
    wtap_block_ref(rec.block);             // Increment rec wtap block ref
    *g_wtap_packet_direction = packet_dir; // Write packet direction to block packet flags pointer;

    // Run actual packet dissection here
    epan_dissect_run(&edt, cfile.cd_t, &rec,
                     frame_tvbuff_new(&cfile.provider, &fdata, raw_packet),
                     &fdata, &cfile.cinfo);

    frame_data_set_after_dissect(&fdata, &cum_bytes);

    prev_dis_frame = fdata;
    cfile.provider.prev_dis = &prev_dis_frame;

    prev_cap_frame = fdata;
    cfile.provider.prev_cap = &prev_cap_frame;

    frame_data_destroy(&fdata);

    // epan_dissect_cleanup(&edt); // Cleanup (not necessary)
    wtap_rec_cleanup(&rec);
}

const char *packet_show()
{
    static char buffer[16384]; // Yes, asn1 generated structure is over the top

    FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
    print_stream_t *print_text = print_stream_text_stdio_new(stream);

    if (!full_dissection) {
        epan_initialized = FALSE;
        full_dissection = TRUE;
        packet_dissect((uint8_t *)edt.tvb->real_data, edt.tvb->length);
        full_dissection = FALSE;
    }

    proto_tree_print(print_dissections_expanded, FALSE, &edt, NULL, print_text);

    fclose(stream);

    full_dissection = FALSE;

    return buffer;
}

const char *packet_show_pdml()
{
    static char buffer[64384]; // Yes, asn1 generated structure is way over the top
    static output_fields_t *output_fields = NULL;

    if (output_fields == NULL)
        output_fields = output_fields_new();

    FILE *stream = fmemopen(buffer, sizeof(buffer), "w");

    if (!full_dissection) {
        epan_initialized = FALSE;
        full_dissection = TRUE;
        packet_dissect((uint8_t *)edt.tvb->real_data, edt.tvb->length);
        full_dissection = FALSE;
    }

    write_pdml_preamble(stream, cfile.filename);
    // write_psml_columns(&edt, stream, FALSE);
    write_pdml_proto_tree(output_fields, NULL, PF_NONE, &edt, &cfile.cinfo, stream, FALSE);
    write_pdml_finale(stream);

    full_dissection = FALSE;

    fclose(stream);

    return buffer;
}

char *packet_description()
{
    static char buffer[8192];

    FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
    print_stream_t *print_text = print_stream_text_stdio_new(stream);

    if (!full_dissection) {
        epan_initialized = FALSE;
        full_dissection = TRUE;
        packet_dissect((uint8_t *)edt.tvb->real_data, edt.tvb->length);
        full_dissection = FALSE;
    }

    proto_tree_print(print_dissections_collapsed, FALSE, &edt, NULL, print_text);
    fclose(stream);

    full_dissection = FALSE;

    return buffer;
}

const char *packet_summary()
{
    return col_get_text(edt.pi.cinfo, COL_INFO);
}

uint8_t packet_direction()
{
    switch (packet_dir) {
    case PACK_FLAGS_DIRECTION_INBOUND:
        return P2P_DIR_RECV;
        break;
    case PACK_FLAGS_DIRECTION_OUTBOUND:
        return P2P_DIR_SENT;
        break;

    default:
        return P2P_DIR_UNKNOWN;
        break;
    }
}

uint32_t packet_layers_count()
{
    epan_dissect_t *edt = wdissector_get_edt();
    proto_tree *node = edt->tree;
    proto_tree *subnode = NULL, *subnode_parent;
    uint32_t layers_count = 0;

    // 1. Root Tree Children (Layers) (Loop)
    node = node->first_child;
    while (node != NULL) {
        layers_count++;

        // 2. Node Children(Fields or other layers)(Loop)
        // printf("==== Layer: %s, Type=%s, Size=%d\n", node->finfo->hfinfo->name, node->finfo->value.ftype->name, node->finfo->length);

        if ((subnode = node->first_child)) {

            while (subnode != NULL) {
                // 3.1 Field Group (Intermediary Node)
                if (subnode->first_child != NULL) {
                    // Only process groups not skipped
                    if ((subnode->finfo->length || subnode->first_child->finfo->length)) {

                        if (subnode->finfo->length) // Only process groups with fields not skipped
                        {
                            if (subnode->finfo->value.ftype->ftype == FT_PROTOCOL || subnode->parent->parent == NULL) {
                                // printf("==== Layer: %s, Type=%s, Size=%d\n", subnode->finfo->hfinfo->name, subnode->finfo->value.ftype->name, subnode->finfo->length);
                                layers_count++;
                            }
                        }
                        // subnode_parent = subnode;
                        subnode = subnode->first_child;
                    }
                    else {
                        // Skip current group
                        if (subnode->next)
                            subnode = subnode->next;
                        else {
                            // Return to previous parent recursivelly
                            // puts("<--- Return");
                            subnode = subnode->parent;
                            while (subnode->next == NULL) {
                                if (!subnode->parent) {
                                    // detect final subnode
                                    subnode = NULL;
                                    node = NULL;
                                    break;
                                }

                                // puts("<--- Return");
                                subnode = subnode->parent;
                            }
                            if (subnode)
                                subnode = subnode->next;
                        }
                    }
                }
                // 3.2 Leaf (Child Node)
                else if (subnode->next != NULL) {
                    subnode = subnode->next;
                }
                // 3.3 Final Leaf (Last Child Node)
                else {
                    // Return to previous parent recursivelly
                    subnode = subnode->parent;
                    while (subnode->next == NULL) {
                        if (!subnode->parent) {
                            // detect final subnode
                            subnode = NULL;
                            node = NULL;
                            break;
                        }

                        // puts("<--- Return");
                        subnode = subnode->parent;
                    }
                    if (subnode)
                        subnode = subnode->next;
                }
            }
        }

        if (node)
            node = node->next;
    }

    return layers_count - 1;
}

int packet_dissectors_count()
{ // discount the frame and user dlt from the count
    guint count = wmem_list_count(edt.pi.layers);
    return (count >= 2 ? count - 2 : 0);
}

// return the name of the later given its index, starting after the first layer (frame)
const char *packet_layer(uint8_t layer_index)
{

    if (layer_index >= wmem_list_count(edt.pi.layers) - 2)
        return NULL;

    wmem_list_frame_t *protocol_layer = wmem_list_head(edt.pi.layers);
    protocol_layer = wmem_list_frame_next(protocol_layer);

    // Iterate trough protocol layers
    for (uint8_t i = 0; i <= layer_index; i++) {
        protocol_layer = wmem_list_frame_next(protocol_layer);
    }
    // Get identification of protocol layer
    int protocol_id = GPOINTER_TO_INT(wmem_list_frame_data(protocol_layer));
    // Get name of layer given its identification number
    return proto_get_protocol_filter_name(protocol_id);
}

static inline char *str_copy_and_advance(const char *src_str, char *dest_str, char append_caracter)
{
    dest_str = stpcpy(dest_str, src_str);
    if (append_caracter) {
        *(dest_str++) = ' ';
        *(dest_str++) = '/';
        *(dest_str++) = ' ';
    }

    return dest_str;
}

const char *packet_dissectors()
{
    static gchar local_text[256]; // fixed buffer to print layers
    char *idx = local_text;

    wmem_list_frame_t *protocol_layer = wmem_list_head(edt.pi.layers);
    int layers_count = packet_dissectors_count();
    protocol_layer = wmem_list_frame_next(protocol_layer);

    while (layers_count--) {
        protocol_layer = wmem_list_frame_next(protocol_layer);
        int protocol_id = GPOINTER_TO_INT(wmem_list_frame_data(protocol_layer));
        const char *layer_name = proto_get_protocol_filter_name(protocol_id);

        idx = str_copy_and_advance(layer_name, idx, '/');
    }

    *(idx - 3) = 0;
    return local_text;
}

const char *packet_relevant_fields()
{
    static gchar local_text[256]; // fixed buffer to print layers
    char *text = local_text;

    if (!full_dissection) {
        epan_initialized = FALSE;
        full_dissection = TRUE;
        packet_dissect((uint8_t *)edt.tvb->real_data, edt.tvb->length);
        full_dissection = FALSE;
    }

    proto_node *p_tree = edt.tree->first_child->next;
    proto_node *p_sub_node;

    while (p_tree) {

        text = str_copy_and_advance(p_tree->finfo->hfinfo->abbrev, text, '/');

        if (p_tree->first_child) {
            p_sub_node = p_tree->first_child;
            while (p_sub_node) {
                text = str_copy_and_advance(p_sub_node->finfo->hfinfo->abbrev, text, '/');
                p_sub_node = p_sub_node->next;
            }
        }

        p_tree = p_tree->next;
    }
    *(text - 3) = 0; // truncate the additional slash after last field/layer name
    return local_text;
}

const char *packet_protocol()
{
    return col_get_text(edt.pi.cinfo, COL_PROTOCOL);
}

void packet_cleanup()
{
    if (!epan_initialized)
        return;
    // wtap_rec_cleanup(&rec);
    epan_dissect_cleanup(&edt);
    epan_free(cfile.epan);

    cfile.epan = raw_epan_new(&cfile);
    cfile.provider.wth = NULL;
    cfile.f_datalen = 0; /* not used, but set it anyway */
    epan_initialized = 0;
    // wtap_rec_init(&rec);
}

void packet_navigate(uint32_t skip_layers, uint32_t skip_groups, uint8_t (*callback)(proto_tree *, uint8_t, uint8_t *))
{
    proto_tree *node = edt.tree;
    proto_tree *subnode = NULL, *subnode_parent;
    uint32_t layers_count = 0;
    uint32_t groups_count = 0;
    uint32_t level = 0;

    if (!node || !callback)
        return;

    // 1. Root Tree Children (Layers) (Loop)
    node = node->first_child;
    while (node != NULL) {
        layers_count++;

        if (layers_count > skip_layers) {
            // 2. Node Children(Fields or other layers)(Loop)
            callback(node, WD_TYPE_LAYER, (uint8_t *)edt.tvb->real_data);
            // printf("==== Layer: %s, Type=%s, Size=%d\n", node->finfo->hfinfo->name, node->finfo->value.ftype->name, node->finfo->length);

            if ((subnode = node->first_child)) {
                level++;

                while (subnode != NULL) {
                    // 3.1 Field Group (Intermediary Node)
                    if (subnode->first_child != NULL) {
                        groups_count++;
                        // Only process groups not skipped
                        if ((groups_count > skip_groups) && (1 || subnode->finfo->length || subnode->first_child->finfo->length)) {

                            if (subnode->finfo->length) // Only process groups with fields not skipped
                            {
                                if (subnode->finfo->value.ftype->ftype == FT_PROTOCOL || subnode->parent->parent == NULL)
                                    callback(subnode, WD_TYPE_LAYER, (uint8_t *)edt.tvb->real_data);
                                // printf("==== Layer: %s, Type=%s, Size=%d\n", subnode->finfo->hfinfo->name, subnode->finfo->value.ftype->name, subnode->finfo->length);
                                else if (subnode->first_child->finfo->length)
                                    callback(subnode, WD_TYPE_GROUP, (uint8_t *)edt.tvb->real_data);
                                // printf("---> Field Group: %s, Type=%s, Size=%d\n", subnode->finfo->hfinfo->name, subnode->finfo->value.ftype->name, subnode->finfo->length);
                                else
                                    // Group with a children without size is actually a field
                                    callback(subnode, WD_TYPE_FIELD, (uint8_t *)edt.tvb->real_data);
                                // printf("Field: %s, Size=%d, Type=%s, Offset=%d\n", subnode->finfo->hfinfo->name, subnode->finfo->length, subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo));
                            }
                            // subnode_parent = subnode;
                            level++;
                            subnode = subnode->first_child;
                        }
                        else {
                            // Skip current group
                            if (subnode->next)
                                subnode = subnode->next;
                            else {
                                // Return to previous parent recursivelly
                                // puts("<--- Return");
                                level--;
                                subnode = subnode->parent;
                                while (subnode->next == NULL) {
                                    if (!subnode->parent) {
                                        // detect final subnode
                                        subnode = NULL;
                                        node = NULL;
                                        break;
                                    }

                                    // puts("<--- Return");
                                    level--;
                                    subnode = subnode->parent;
                                }
                                if (subnode)
                                    subnode = subnode->next;
                            }
                        }
                    }
                    // 3.2 Leaf (Child Node)
                    else if (subnode->next != NULL) {
                        if (subnode->finfo->length && subnode->finfo->value.ftype->ftype != FT_PROTOCOL && subnode->finfo->value.ftype->ftype != FT_NONE) // Only process fields not skipped
                            callback(subnode, WD_TYPE_FIELD, (uint8_t *)edt.tvb->real_data);
                        // printf("Field: %s, Size=%d, Type=%s, Offset=%d\n", subnode->finfo->hfinfo->name, subnode->finfo->length, subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo));

                        subnode = subnode->next;
                    }
                    // 3.3 Final Leaf (Last Child Node)
                    else {
                        if (subnode->finfo->length && subnode->finfo->value.ftype->ftype != FT_NONE) // Only process fields not skipped
                            callback(subnode, WD_TYPE_FIELD, (uint8_t *)edt.tvb->real_data);
                        // printf("Field: %s, Size=%d, Type=%s, Offset=%d\n", subnode->finfo->hfinfo->name, subnode->finfo->length, subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo));

                        // Return to previous parent recursivelly
                        // puts("<--- Return");
                        level--;
                        subnode = subnode->parent;
                        while (subnode->next == NULL) {
                            if (!subnode->parent) {
                                // detect final subnode
                                subnode = NULL;
                                node = NULL;
                                break;
                            }

                            // puts("<--- Return");
                            level--;
                            subnode = subnode->parent;
                        }
                        if (subnode)
                            subnode = subnode->next;
                    }
                }
            }
        }

        if (node)
            node = node->next;
    }
}

static uint8_t test_navigate_callback(proto_tree *subnode, uint8_t field_type, uint8_t *pkt_buf)
{

    uint64_t raw_val;
    int64_t parsed_val;
    uint64_t mask;
    uint8_t align_to_octet = 0;
    uint32_t field_length = subnode->finfo->length;

    switch (field_type) {
    case WD_TYPE_FIELD:
        // --------------- Raw value ---------------
        // Assume field is 8 bytes (ignore read-only overflow for performance)
        raw_val = *(uint64_t *)&pkt_buf[packet_read_field_offset(subnode->finfo)];

        // print raw_val in hexadecimal

        if (subnode->finfo->ds_tvb->bitshift_from_octet != 0) {
            align_to_octet = subnode->finfo->ds_tvb->bitshift_from_octet;
            if (align_to_octet > 0) {
                raw_val >>= align_to_octet;
            }
            else if (align_to_octet < 0) {
                raw_val <<= -align_to_octet;
            }

            ++field_length;
        }

        // Check if format is BIG_ENDIAN and swap bytes for raw value
        if ((field_length > 1) &&
                (packet_read_field_encoding(subnode->finfo) == FI_BIG_ENDIAN) ||
            (align_to_octet > 0)) {
            switch (field_length) {
            case 2:
                // puts("__bswap_16");
                raw_val = __bswap_16(raw_val);
                break;
            case 4:
                // puts("__bswap_32");
                raw_val = __bswap_32(raw_val);
                break;
            case 8:
                // puts("__bswap_64");
                raw_val = __bswap_64(raw_val);
                break;
            default: {
                if (field_length > 8)
                    break;
                puts("arbitrary length");
                // Swap bytes for raw value to big endian
                uint64_t tmp;
                uint8_t *p = (uint8_t *)&tmp;
                uint8_t *q = (uint8_t *)&raw_val;
                int i;
                for (i = 0; i < field_length; i++)
                    p[i] = q[field_length - i - 1];
                raw_val = tmp;
            } break;
            }
        }

        printf("L=%d, val: 0x%016" PRIx64 "\n", field_length, raw_val);

        if (align_to_octet)
            --field_length;

        mask = packet_read_field_bitmask(subnode->finfo);

        if (mask) {
            raw_val = (raw_val & mask) >> packet_read_field_bitmask_offset(mask); // For fields that have mask
        }
        else {
            raw_val &= (UINT64_MAX) >> (64 - (field_length << 3)); // For fields without mask (val << 3 means val * 8)
        }

        // --------------- Parsed value ---------------
        parsed_val = packet_read_field_int64(subnode->finfo);

        printf("     Field: %s, Size=%u, Type=%s,%s, Offset=%u, Mask=%lX, Bit=%d, Value=%ld, Min:%ld, Max=%ld, Raw Value:%X,\n",
               subnode->finfo->hfinfo->name,
               field_length,
               (packet_read_field_encoding(subnode->finfo) == FI_BIG_ENDIAN) ? "B" : "L",
               subnode->finfo->value.ftype->name,
               packet_read_field_offset(subnode->finfo),
               mask,
               packet_read_field_bitmask_offset(mask),
               parsed_val, // TODO: FT_BYTES will print garbage
               subnode->finfo->value_min,
               subnode->finfo->value_max,
               raw_val);
        break;

    case WD_TYPE_GROUP:
        printf("\033[36m"
               "---> Group: %s, Type=%s,%s, Size=%d\n"
               "\033[00m",
               subnode->finfo->hfinfo->name,
               (packet_read_field_encoding(subnode->finfo) == FI_BIG_ENDIAN) ? "B" : "L",
               subnode->finfo->value.ftype->name,
               subnode->finfo->length);
        break;
    case WD_TYPE_LAYER:
        printf("\033[33m"
               "==== Layer: %s, Type=%s, Size=%d\n"
               "\033[00m",
               subnode->finfo->hfinfo->name,
               subnode->finfo->value.ftype->name,
               subnode->finfo->length);
        break;

    default:
        break;
    }

    return 0;
}

void packet_benchmark(gboolean detailed)
{
#if ENABLE_PROFILING == TRUE

    for (uint32_t i = 0; i < 1000; i++) {
        // packet_dissect(DEMO_PKT_RRC_CONNECTION_SETUP+34, 66); // udp
        packet_dissect(DEMO_PKT_RRC_RECONFIGURATION + 49, sizeof(DEMO_PKT_RRC_RECONFIGURATION) - 49);
    }

    PROFILING_LOOP_START;
    // packet_dissect(DEMO_PKT_RRC_CONNECTION_SETUP+34, 66); // udp
    packet_dissect(DEMO_PKT_RRC_RECONFIGURATION + 49, sizeof(DEMO_PKT_RRC_RECONFIGURATION) - 49); // mac-lte-framed
    PROFILING_LOOP_STOP("packet_dissect");

    PROFILING_LOOP_START;
    packet_get_field("mac-lte.length");
    PROFILING_LOOP_STOP("packet_get_field");

    PROFILING_LOOP_START;
    packet_has_condition("lte-rrc.rrc_TransactionIdentifier==1 && lte-rrc.sr_ConfigIndex==5");
    PROFILING_LOOP_STOP("packet_has_condition");

    packet_register_condition("lte-rrc.rrc_TransactionIdentifier==1 && lte-rrc.sr_ConfigIndex==5", 0);
    PROFILING_LOOP_START;
    packet_set_condition(0);
    packet_dissect(DEMO_PKT_RRC_RECONFIGURATION + 49, sizeof(DEMO_PKT_RRC_RECONFIGURATION) - 49);
    packet_read_condition(0);
    PROFILING_LOOP_STOP("packet_has_condition (cached)");

#if ENABLE_PROFILING_HISTOGRAM == TRUE
    if (detailed) {
        printf("packet_has_condition\n");
        for (uint32_t i = 0; i < 1000; i++) {
            printf("%04d ", i);
            packet_dissect(DEMO_PKT_RRC_RECONFIGURATION + 49, sizeof(DEMO_PKT_RRC_RECONFIGURATION) - 49);
            PROFILING_LOOP_START;
            // packet_navigate(1, 0, test_callback);
            packet_has_condition("lte-rrc.rrc_TransactionIdentifier==1 && lte-rrc.sr_ConfigIndex==5");
            PROFILING_LOOP_STOP("packet_has_condition");
            packet_cleanup();
        }

        printf("packet_has_condition (cached)\n");
        for (uint32_t i = 0; i < 1000; i++) {
            printf("%04d ", i);
            packet_register_condition("lte-rrc.rrc_TransactionIdentifier==1 && lte-rrc.sr_ConfigIndex==5", 0);
            PROFILING_LOOP_START;
            packet_set_condition(0);
            packet_dissect(DEMO_PKT_RRC_RECONFIGURATION + 49, sizeof(DEMO_PKT_RRC_RECONFIGURATION) - 49);
            packet_has_condition(NULL);
            PROFILING_LOOP_STOP("packet_has_condition (cached)");
            packet_cleanup();
        }
        exit(0);
    }
#endif

#endif
}

const char *packet_register_filter(const char *filter)
{
    const char *compiled_filter;
    if (dfilter_compile(filter, (dfilter_t **)&compiled_filter, NULL))
        return compiled_filter;
    else
        return NULL;
}

void packet_set_filter(const char *filter)
{
    if (edt.pi.data_src) {
        epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
    }
    epan_dissect_prime_with_dfilter(&edt, (const struct epan_dfilter *)filter);
}

gboolean packet_read_filter(const char *filter)
{
    if (filter != NULL) {
        // If filter is enabled and rfcode is registered
        return dfilter_apply_edt((dfilter_t *)filter, &edt);
    }
    return FALSE;
}

void packet_clean_filter(const char **compiled_filter)
{
    if (compiled_filter && *compiled_filter) {
        dfilter_free(*((dfilter_t **)compiled_filter));
        *compiled_filter = NULL;
    }
}

gboolean packet_register_condition(const char *filter, uint16_t condition_index)
{
    if (filtercodes[condition_index] != NULL) {
        dfilter_free(filtercodes[condition_index]);
    }
    return dfilter_compile(filter, &filtercodes[condition_index], NULL);
}

void packet_set_condition(uint16_t condition_index)
{
    if (filtercodes[filtercode_index] != NULL) {
        filtercode_index = condition_index;

        if (edt.pi.data_src) {
            epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
        }
        epan_dissect_prime_with_dfilter(&edt, filtercodes[filtercode_index]);
    }
}

gboolean packet_read_condition(uint16_t condition_index)
{

    if (filtercodes[condition_index] != NULL) {
        // If filter is enabled and rfcode is registered
        return dfilter_apply_edt(filtercodes[condition_index], &edt);
    }

    return FALSE;
}

gboolean packet_has_condition(const char *filter)
{
    if (!edt.tvb)
        return FALSE;
    // [Method 2] Compile and use a filter
    dfilter_t *rfcode;
    gboolean result;

    if (filter != NULL) {
        if (dfilter_compile(filter, &rfcode, NULL)) {

            uint8_t *pkt_buf = (uint8_t *)edt.tvb->real_data;
            uint32_t pkt_len = edt.tvb->length;

            if (edt.pi.data_src) {
                epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
            }
            epan_dissect_prime_with_dfilter(&edt, rfcode);
            packet_dissect(pkt_buf, pkt_len);
            result = dfilter_apply_edt(rfcode, &edt); // return filter value (true or false)
            dfilter_free(rfcode);
            return result;
        }
    }
    else if (filtercodes[filtercode_index] != NULL) {
        // If filter is enabled and rfcode is registered
        result = dfilter_apply_edt(filtercodes[filtercode_index], &edt);

        return result;
    }

    return FALSE;
}

gboolean packet_register_field(const char *field_name, uint16_t field_hfi_index)
{
    if (field_name != NULL) {
        header_field_info *hfi = proto_registrar_get_byname(field_name);

        if (hfi == NULL) {
            return FALSE; // Field was not found
        }
        // printf("%s:%ld\n", field_name, hfi->id);
        fields_hfi[fields_hfi_index] = hfi;
        return TRUE;
    }

    return FALSE;
}

void packet_set_field(uint16_t hfi_index)
{
    header_field_info *hfi = fields_hfi[hfi_index];

    if (hfi) {
        fields_hfi_index = hfi_index;
        if (edt.pi.data_src) {
            epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
        }
        epan_dissect_prime_with_hfid(&edt, hfi->id);
        // do
        // {
        //     // Mark field in the tree

        //     hfi = hfi->same_name_next;
        // } while (hfi != -1);
    }
}

void packet_register_set_field(const char *field_name, uint16_t field_hfi_index)
{
    packet_register_field(field_name, field_hfi_index);
    packet_set_field(field_hfi_index);
}

// Return first match of the field
field_info *packet_read_field(uint16_t hfi_index)
{

    GPtrArray *pointers = proto_get_finfo_ptr_array(edt.tree, fields_hfi[hfi_index]->id);

    if (g_ptr_array_len(pointers) > 0) {
        // return first match of field info
        return (field_info *)pointers->pdata[0];
    }

    return NULL;
}

// Return all matches of the field
GPtrArray *packet_read_fields(uint16_t hfi_index)
{
    if (fields_hfi[hfi_index])
        return proto_get_finfo_ptr_array(edt.tree, fields_hfi[hfi_index]->id);
    else
        return NULL;
}

field_info *packet_read_field_at(GPtrArray *fields, uint16_t idx)
{
    return (field_info *)fields->pdata[idx];
}

// -------------
header_field_info *packet_register_set_field_hfinfo(const char *field_name)
{
    header_field_info *hfi = packet_get_header_info(field_name);
    if (hfi)
        packet_set_field_hfinfo(hfi);
    return hfi;
}

void packet_set_field_hfinfo(header_field_info *hfi)
{
    if (edt.pi.data_src) {
        epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
    }
    epan_dissect_prime_with_hfid(&edt, hfi->id);
}

// TODO: measure impact of packet_set_field_hfinfo_all and add into packet_set_field_hfinfo
// Possibly by exposing proto_tree_prime_with_hfid here
void packet_set_field_hfinfo_all(header_field_info *hfi)
{
    if (edt.pi.data_src) {
        epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
    }

    while (1) {
        epan_dissect_prime_with_hfid(&edt, hfi->id);
        if (hfi->same_name_prev_id != -1)
            hfi = proto_registrar_get_nth(hfi->same_name_prev_id);
        else
            return;
    }
}

int packet_read_field_exists_hfinfo(header_field_info *hfi)
{
    return ((int *)packet_read_field_hfinfo(hfi) > (int *)0 ? 1 : 0);
}

// Return first match of the field. Such match iterates over all fields with the same name
// Important: All Fields with same name must be pruned
field_info *packet_read_field_hfinfo(header_field_info *hfi)
{
    while (true) {
        GPtrArray *pointers = (GPtrArray *)g_hash_table_lookup(PTREE_DATA(edt.tree)->interesting_hfids,
                                                               GINT_TO_POINTER(hfi->id));

        if (g_ptr_array_len(pointers) > 0) {
            // return first match of field info
            return (field_info *)pointers->pdata[0];
        }
        // Check id of next field with same name
        if (hfi->same_name_prev_id != -1)
            hfi = proto_registrar_get_nth(hfi->same_name_prev_id);
        else
            return NULL;
    }

    return NULL;
}

GPtrArray *packet_read_fields_hfinfo(header_field_info *hfi)
{
    if (hfi)
        return proto_get_finfo_ptr_array(edt.tree, hfi->id);
    else
        return NULL;
}

// -------------

const char *packet_read_field_name(field_info *field_match)
{
    if (field_match)
        return field_match->hfinfo->name;
    else
        return NULL;
}

const char *packet_read_field_abbrev(field_info *field_match)
{
    if (field_match)
        return field_match->hfinfo->abbrev;
    else
        return NULL;
}

uint16_t packet_read_field_offset(field_info *field_match)
{

    uint32_t offset = field_match->start + field_match->ds_tvb->offset_from_parent;
    tvbuff_t *tvbuff_parent = field_match->ds_tvb->parent;

    // printf("1) field_match->ds_tvb->offset_from_parent=%d\n", field_match->ds_tvb->offset_from_parent);

    while ((tvbuff_parent != NULL) && (tvbuff_parent->raw_offset == 0)) {
        offset += tvbuff_parent->offset_from_parent;
        tvbuff_parent = tvbuff_parent->parent;
    }

    // puts("---------------------");
    // printf("1) field_match->start=%d\n", field_match->start);
    // printf("1) field_match->ds_tvb->offset_from_parent=%d\n", field_match->ds_tvb->offset_from_parent);

    // // if (field_match->ds_tvb->parent) {
    // //     printf("--------Parent--------\n");
    // //     tvbuff_parent = field_match->ds_tvb->parent;
    // //     printf("2) tvbuff_parent->raw_offset=%d\n", tvbuff_parent->raw_offset);
    // //     printf("2) tvbuff_parent->offset_from_parent=%d\n", tvbuff_parent->offset_from_parent);
    // //     tvbuff_parent = tvbuff_parent->parent;
    // //     if (tvbuff_parent) {
    // //         printf("3) tvbuff_parent->raw_offset=%d\n", tvbuff_parent->raw_offset);
    // //         printf("3) tvbuff_parent->offset_from_parent=%d\n", tvbuff_parent->offset_from_parent);

    // //         tvbuff_parent = tvbuff_parent->parent;
    // //         if (tvbuff_parent) {
    // //             printf("4) tvbuff_parent->raw_offset=%d\n", tvbuff_parent->raw_offset);
    // //             printf("4) tvbuff_parent->offset_from_parent=%d\n", tvbuff_parent->offset_from_parent);
    // //         }
    // //     }
    // // }

    return offset;
}

uint32_t packet_read_field_size(field_info *field_match)
{
    return field_match->length;
}

uint8_t packet_read_field_size_bits(uint64_t bitmask)
{
    // return FI_GET_BITS_SIZE(field_match);
    if (bitmask > 0)
        return abs(__bsfq(bitmask) - __bsrq(bitmask)) + 1;
    else
        return 0;
}

uint64_t packet_read_field_bitmask(field_info *field_match)
{
    int bit_size = FI_GET_BITS_SIZE(field_match);
    if (bit_size) // unaligned bit fields (not per)
    {
        uint64_t new_mask = 0xffffffffffffffffUL >> bit_size;
        new_mask = ~new_mask; // Create mask at bit offset 0 msb

        return new_mask >> (64 - (field_match->length << 3) +
                            FI_GET_BITS_OFFSET(field_match));
    }
    // TODO: handle > 64 bits unaligned FT_BYTES
    return field_match->hfinfo->bitmask;
}

uint8_t packet_read_field_bitmask_offset(uint64_t bitmask)
{
    if (G_UNLIKELY(!bitmask))
        return 0;

    // Find the first set bit starting from the lsb.
    return __bsfq(bitmask);
}

uint8_t packet_read_field_bitmask_offset_msb(uint64_t bitmask)
{
    if (G_UNLIKELY(!bitmask))
        return 0;

    // Find the first set bit starting from the lsb.
    return __bsrq(bitmask);
}

unsigned char *packet_read_field_ustring(field_info *field_match)
{
    return field_match->value.value.ustring;
}

GByteArray *packet_read_field_bytes(field_info *field_match)
{
    return field_match->value.value.bytes;
}

int packet_read_field_type(field_info *field_match)
{
    return (int)field_match->value.ftype->ftype;
}

const char *packet_read_field_type_name(field_info *field_match)
{
    return field_match->value.ftype->name;
}

uint32_t packet_read_field_encoding(field_info *field_match)
{
    if (G_UNLIKELY(!field_match->source_tree))
        return 0;

    return field_match->source_tree->finfo->flags & 0x18;
}

const char *packet_read_field_encoding_name(field_info *field_match)
{
    uint32_t flags = packet_read_field_encoding(field_match);

    if (G_UNLIKELY(!flags))
        return "unknown";

    return (flags & FI_BIG_ENDIAN ? "FI_BIG_ENDIAN" : "FI_LITTLE_ENDIAN");
}

const char *packet_read_field_string(field_info *field_match)
{
    return fvalue_to_string_repr(NULL, &field_match->value, FTREPR_DISPLAY, field_match->hfinfo->display);
}

uint32_t packet_read_field_uint32(field_info *field_match)
{
    return field_match->value.value.uinteger;
}

int32_t packet_read_field_int32(field_info *field_match)
{
    return field_match->value.value.sinteger;
}

uint64_t packet_read_field_uint64(field_info *field_match)
{
    return field_match->value.value.uinteger64;
}

int64_t packet_read_field_int64(field_info *field_match)
{
    return field_match->value.value.sinteger64;
}

header_field_info *packet_get_header_info(const char *field_name)
{
    header_field_info *hfi = proto_registrar_get_byname(field_name);
    if (hfi)
        return hfi;
    else
        return NULL;
}

int packet_get_field_exists(const char *field_name)
{
    return ((int *)packet_get_field(field_name) > (int *)0 ? 1 : 0);
}

field_info *packet_get_field(const char *field_name)
{
    if (!edt.tvb)
        return NULL;

    header_field_info *hfi = proto_registrar_get_byname(field_name);

    // puts(((header_field_info *)hfi->parent)->name);

    if (hfi) {
        uint8_t *pkt_buf = (uint8_t *)edt.tvb->real_data;
        uint32_t pkt_len = edt.tvb->length;

        // [Method 1] Prime tree if the header information id
        if (edt.pi.data_src) {
            epan_dissect_reset(&edt); // reset here, so packet_dissect ignore it later
        }

        epan_dissect_prime_with_hfid(&edt, hfi->id); // Mark field in the tree

        packet_dissect(pkt_buf, pkt_len); // Dissect again (yes, it's faster to dissect again if field was not primed before)

        GPtrArray *pointers = proto_get_finfo_ptr_array(edt.tree, hfi->id);
        // int t = proto_check_field_name(field_name);
        // printf("%s, id: %d, count: %d, t:%d\n", field_name, hfi->id, g_ptr_array_len(pointers), t);

        // GPtrArray *pointers = proto_find_finfo(edt.tree, hfi->id);
        if (g_ptr_array_len(pointers) > 0) {

            // return first match of field info
            // printf("Len: %d\n", g_ptr_array_len(pointers));
            return (field_info *)pointers->pdata[0]; // TODO: Support multiple matches?
        }
        return NULL;
    }
    return NULL;
}

// Return field name
const char *packet_get_field_name(const char *field_name)
{
    header_field_info *header_match = packet_get_header_info(field_name);

    if (header_match)
        return header_match->name;
    else
        return NULL;
}

uint32_t packet_get_field_uint32(const char *field_name)
{
    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return packet_read_field_uint32(field_match);
    else
        return 0;
}

const char *packet_get_field_string(const char *field_name)
{
    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return packet_read_field_string(field_match);
    else
        return NULL;
}

uint32_t packet_get_field_size(const char *field_name)
{
    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return packet_read_field_size(field_match);
    else
        return 0;
}

int packet_get_field_type(const char *field_name)
{

    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return (int)field_match->value.ftype->ftype;
    else
        return 0;
}

const char *packet_get_field_type_name(const char *field_name)
{

    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return field_match->value.ftype->name;
    else
        return NULL;
}

const char *packet_get_field_encoding_name(const char *field_name)
{

    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return (field_match->flags & ENC_LITTLE_ENDIAN ? "ENC_LITTLE_ENDIAN" : "ENC_BIG_ENDIAN");
    else
        return NULL;
}

uint32_t packet_get_field_encoding(const char *field_name)
{

    field_info *field_match = packet_get_field(field_name);

    if (field_match)
        return packet_read_field_encoding(field_match);
    else
        return 0;
}

unsigned long packet_get_field_bitmask(const char *field_name)
{
    field_info *field_match = packet_get_field(field_name);

    if (field_match != NULL) {
        int bit_size = FI_GET_BITS_SIZE(field_match);
        if (bit_size) // unaligned bit fields (not per)
        {
            uint64_t new_mask = 0xffffffffffffffffUL >> bit_size;
            new_mask = ~new_mask; // Create mask at bit offset 0 msb

            return new_mask >> (64 - (field_match->length << 3) +
                                FI_GET_BITS_OFFSET(field_match));
        }
        // TODO: handle > 64 bits unaligned FT_BYTES
        return field_match->hfinfo->bitmask;
    }
    else {
        return 0;
    }
}

uint32_t packet_get_field_offset(const char *field_name)
{
    field_info *field_match = packet_get_field(field_name);

    if (field_match != NULL) {
        uint32_t offset = field_match->start;
        proto_tree *parent = field_match->source_tree->parent;

        if (parent && parent->finfo && (offset < parent->finfo->start)) {
            return offset + parent->finfo->start;
        }
        else {
            return offset;
        }
    }
    else {
        return 0;
    }
}

// Print field summary (length, offset, encoding, bitmask, etc)
uint8_t packet_field_summary(uint8_t *raw_packet, uint32_t packet_length, const char *field_name)
{

    packet_register_set_field(field_name, 0); // Register and set target field to index 0
    packet_dissect(raw_packet, packet_length);
    printf("Summary: %s\n", packet_summary());

    GPtrArray *fields = packet_read_fields(0); // Search and get fields info array by cached index 0
    //    GPtrArray *fields = proto_get_finfo_ptr_array(edt.tree, packet_get_header_info(field_name)->id); // Search and get fields info array by cached index 0
    // GPtrArray *fields = proto_find_finfo(edt.tree, packet_get_header_info(field_name)->id); // proto_find_finfo
    field_info *field;
    // Iterate over all security header type fields
    if (fields) {
        printf("[ Field: %s ]\n", field_name);
        for (int i = 0; i < fields->len; i++) {
            uint64_t mask, offset, length;
            field = packet_read_field_at(fields, i);
            printf("----> Match %d\n", i);
            printf("Encoding: %s\n", packet_read_field_encoding_name(field));
            printf("Offset: %ld\n", offset = packet_read_field_offset(field));
            printf("Value: %s\n", packet_read_field_string(field));
            printf("Length: %ld\n", length = packet_read_field_size(field));
            printf("Type: %s (%d)\n", packet_read_field_type_name(field), packet_read_field_type(field));
            printf("Bitmask: 0x%04lx\n", mask = packet_read_field_bitmask(field));
            printf("Bitmask Offset: %d\n", packet_read_field_bitmask_offset(mask));
            printf("Bitmask Length: %d\n", packet_read_field_size_bits(mask));
            printf("Raw bytes: 0x");
            for (size_t i = offset; i < offset + length; i++) {
                printf("%02X", raw_packet[i]);
            }
            printf("\n");

            printf("Raw bits offset: %d\n", FI_GET_BITS_OFFSET(field));
            printf("Raw bits size: %d\n", FI_GET_BITS_SIZE(field));
        }
        return 0;
    }
    return -1;
}

// main is not declared when building this code as static library
#ifndef WDISSECTOR_STATIC_LIB
int main(int argc, char **argv)
{
    wdissector_init("encap:1");                // Initialize library
    wdissector_enable_fast_full_dissection(1); // Enable dissection of all fields without string conversion

    // wdissector_init(NULL); // initialize with default wireshark dissector encap:1 (ethernet)
    // packet_set_protocol("proto:udp"); // use offset 34 for udp (heuristics - slower)
    // packet_set_protocol("proto:mac-lte-framed"); // use offset 49 for mac-lte-framed (no udp)

    // packet_benchmark(argc > 1 && *argv[1] == 'b');

    printf("------- Packet Details -----------------\n");
    uint8_t packet_mac_rrc_nas[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
                                    0xa2, 0x2d, 0x73, 0x40, 0x0, 0x40, 0x11, 0x0,
                                    0x0, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
                                    0x1, 0xe7, 0xa6, 0x27, 0xf, 0x0, 0x8e, 0xfe,
                                    0x9a, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
                                    0x1, 0x3, 0x2, 0x0, 0x73, 0x7, 0x1, 0x1f,
                                    0x0, 0x4, 0x1, 0x41, 0x0, 0x35, 0xc0, 0x0,
                                    0x0, 0x0, 0x2c, 0x85, 0x4f, 0xc0, 0xa, 0xc0,
                                    0x0, 0x40, 0x0, 0x4, 0x2d, 0x98, 0xef, 0x8c,
                                    0xa7, 0xb0, 0xa, 0x96, 0x43, 0x6e, 0xd7, 0x5b,
                                    0x8, 0xb1, 0x12, 0x2b, 0xa4, 0x2, 0xb, 0xec,
                                    0xcc, 0xd2, 0x8a, 0xdf, 0x30, 0x0, 0x13, 0x73,
                                    0x33, 0xad, 0x58, 0xfe, 0x59, 0xc0, 0x40, 0x0,
                                    0x0, 0x0, 0x0, 0x3f, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    packet_set_direction(P2P_DIR_RECV);
    packet_dissect(packet_mac_rrc_nas, sizeof(packet_mac_rrc_nas));
    printf("Summary: %s\n", packet_summary());
    printf("Layers count: %d\n", packet_dissectors_count());
    printf("Layers: [%s]\n", packet_dissectors());
    printf("Layer 1 name: %s\n", packet_layer(1));
    // puts(packet_show());
    packet_navigate(1, 0, test_navigate_callback);

    // printf("\n------- Packet Filtering ---------------\n");
    // // Filtering by string (Slow)
    // printf("Packet has field \"lte-rrc.rrc_TransactionIdentifier\": %s\n", (packet_get_field_exists("lte-rrc.rrc_TransactionIdentifier") > 0 ? "yes" : "no"));
    // printf("Field value: %s\n", packet_get_field_string("lte-rrc.rrc_TransactionIdentifier"));
    // printf("Filter \"lte-rrc.rrc_TransactionIdentifier==1 && lte-rrc.sr_ConfigIndex==5\": %d\n", packet_has_condition("rlc-lte.am.fixed.sn"));
    // // Filtering by registering the filter first (Faster)
    // packet_register_condition("lte-rrc.rrc_TransactionIdentifier==1 && lte-rrc.sr_ConfigIndex==5", 0);
    // packet_set_condition(0);
    // packet_dissect(DEMO_PKT_RRC_CONNECTION_SETUP + 49, sizeof(DEMO_PKT_RRC_CONNECTION_SETUP) - 49);
    // int filter_passes = packet_read_condition(0);
    // printf("Filter (cached) valid: %d\n", filter_passes);

    // printf("\n------- API DEMO (By field name - slower) --------\n");
    // const char *test_field = "lte-rrc.logicalChannelGroup";
    // printf("Offset of %s: %d\n", test_field, packet_get_field_offset(test_field));
    // printf("Value of %s: %s\n", test_field, packet_get_field_string(test_field));
    // printf("Length of %s: %d\n", test_field, packet_get_field_size(test_field));
    // printf("Type of %s: %d (%s)\n", test_field, packet_get_field_type(test_field), packet_get_field_type_name(test_field));
    // printf("Bitmask of %s: 0x%04lx\n", test_field, packet_get_field_bitmask(test_field));
    // printf("Encoding of %s: %s\n", test_field, packet_get_field_encoding_name(test_field));
    // printf("Direction: %d\n", packet_direction());

    // printf("\n------- API DEMO (By field index - faster) --------\n"); // Faster as redissection is not needed
    // packet_field_summary(DEMO_PKT_RRC_CONNECTION_SETUP + 49, sizeof(DEMO_PKT_RRC_CONNECTION_SETUP) - 49, test_field);

    // // See packet_field_summary to handle multiple "equal" fields within a packet
    // printf("\n------- API DEMO (NAS-EPS-FRAMED) --------\n");
    // packet_set_protocol("proto:nas-eps"); // Change protocol to NAS EPS (framed does not need udp)
    // const char *test_nas_field1 = "nas_eps.security_header_type";
    // printf("1) Dissecting Field: %s\n", test_nas_field1);
    // packet_field_summary(DEMO_PKT_NAS_ATTACH_REQUEST, sizeof(DEMO_PKT_NAS_ATTACH_REQUEST), test_nas_field1);
    // printf("\n");
    // const char *test_nas_field2 = "nas_eps.emm.nas_key_set_id";
    // printf("2) Dissecting Field: %s\n", test_nas_field2);
    // packet_field_summary(DEMO_PKT_NAS_ATTACH_REQUEST, sizeof(DEMO_PKT_NAS_ATTACH_REQUEST), test_nas_field2);

    // printf("\n------- API DEMO (BTACL) --------\n");
    // packet_set_protocol("encap:BLUETOOTH_HCI_H4"); // Bluetooth ACL
    // const char *test_bt_field = "btbbd.type";
    // printf("1) Dissecting Field: %s\n", test_bt_field);

    // uint8_t packet[] = {0x9, 0x76, 0x28, 0x0, 0x0, 0x2b, 0x0, 0x99, 0x1,
    //                     0x4f, 0x0, 0x50, 0xff, 0xff, 0x8f, 0xfe, 0xdb,
    //                     0xff, 0x5b, 0x87, 0x49};

    // packet_set_direction(1); // RX Packet
    // packet_field_summary(packet, sizeof(packet), test_bt_field);
    // // packet_navigate(2, 1, test_navigate_callback);

    // {
    //     uint8_t packet[] = {0x9, 0x7d, 0x52, 0x0, 0x0, 0x2d, 0xe, 0xa6, 0x2,
    //                         0x66, 0x0, 0x8, 0x0, 0x1, 0x0, 0x2, 0x4,
    //                         0x4, 0x0, 0x1, 0x0, 0x4f, 0x0, 0x7b};

    //     packet_set_direction(1);
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // {
    //     uint8_t packet[] = {0x9, 0xa9, 0x52, 0x0, 0x0, 0x0, 0x12, 0x1e, 0x0,
    //                         0x86, 0x0, 0xc, 0x0, 0x1, 0x0, 0x3, 0x4,
    //                         0x8, 0x0, 0x41, 0x0, 0x4f, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x5d};

    //     packet_set_direction(0);
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // {
    //     uint8_t packet[] = {0x9, 0xaa, 0x52, 0x0, 0x0, 0x0, 0x12, 0x1e, 0x0,
    //                         0x86, 0x0, 0xc, 0x0, 0x1, 0x0, 0x4, 0x2,
    //                         0x8, 0x0, 0x4f, 0x0, 0x0, 0x0, 0x1, 0x2,
    //                         0x9b, 0x6, 0xc0};

    //     packet_set_direction(0);
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // {
    //     uint8_t packet[] = {0x9, 0xaf, 0x52, 0x0, 0x0, 0x34, 0x2, 0xa6, 0x3,
    //                         0x86, 0x0, 0xc, 0x0, 0x1, 0x0, 0x4, 0x5,
    //                         0x8, 0x0, 0x41, 0x0, 0x0, 0x0, 0x1, 0x2,
    //                         0xa0, 0x2, 0x6a};
    //     packet_set_direction(1);
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // {
    //     uint8_t packet[] = {0x9, 0xb1, 0x52, 0x0, 0x0, 0x31, 0x2, 0xa6, 0x1,
    //                         0x76, 0x0, 0xa, 0x0, 0x1, 0x0, 0x5, 0x2,
    //                         0x6, 0x0, 0x41, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0xac};

    //     packet_set_direction(1);
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // {
    //     uint8_t packet[] = {0x9, 0xb1, 0x52, 0x0, 0x0, 0x0, 0x12, 0x26, 0x0,
    //                         0x96, 0x0, 0xe, 0x0, 0x1, 0x0, 0x5, 0x5,
    //                         0xa, 0x0, 0x4f, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x1, 0x2, 0xa0, 0x2, 0xe8};

    //     packet_set_direction(0);
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // {
    //     uint8_t packet[] = {0x9, 0xb9, 0x52, 0x0, 0x0, 0x41, 0x2, 0xc6, 0x3,
    //                         0xc6, 0x0, 0x14, 0x0, 0x41, 0x0, 0x6, 0x0,
    //                         0x0, 0x0, 0xf, 0x35, 0x3, 0x19, 0x12, 0x0,
    //                         0x2, 0x90, 0x35, 0x5, 0xa, 0x0, 0x0, 0xff,
    //                         0xff, 0x0, 0x7e};

    //     packet_set_direction(1);

    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());
    // }

    // packet_navigate(2, 1, test_navigate_callback);

    // {
    //     uint8_t packet[] = {0x04, 0x2f, 0xff, 0x1, 0x97, 0x0, 0x9e, 0x74, 0x77, 0xfc,
    //                         0x1, 0x0, 0x4, 0x1, 0x2a, 0x23, 0x30, 0xc7,
    //                         0x10, 0x9, 0x44, 0x45, 0x53, 0x4b, 0x54, 0x4f,
    //                         0x50, 0x2d, 0x51, 0x33, 0x50, 0x32, 0x31, 0x4d,
    //                         0x53, 0x2, 0xa, 0xc, 0xd, 0x3, 0xc, 0x11,
    //                         0xa, 0x11, 0x1f, 0x11, 0xe, 0x11, 0xb, 0x11,
    //                         0x1e, 0x11, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    //     // Change protocol to FULL hci
    //     packet_set_protocol("encap:BLUETOOTH_HCI_H4");
    //     packet_dissect(packet, sizeof(packet));
    //     puts(packet_summary());

    //     printf("----- GET Addresse here -----\n");
    //     printf("BDAddress: %s\n", packet_get_field_string("bthci_evt.bd_addr"));
    //     printf("Name: %s\n", packet_get_field_string("btcommon.eir_ad.entry.device_name"));
    //     packet_set_protocol("proto:hci_h4"); // Change back to normal hci_h4
    // }

    // printf("Layers count: %d\n", packet_layers_count());
    // printf("Layers: [%s]\n", packet_dissectors());

    // uint8_t gnb_rar_packet[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                             0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    //                             0x39, 0x2d, 0x5f, 0x40, 0x0, 0x40, 0x11, 0xf,
    //                             0x53, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    //                             0x1, 0xe7, 0xa6, 0x27, 0xf, 0x0, 0x25, 0xfe,
    //                             0x38, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
    //                             0x1, 0x2, 0x2, 0x0, 0x7f, 0x3, 0x0, 0x0,
    //                             0x7, 0x0, 0xc0, 0x0, 0x7, 0x1, 0x7f, 0x0,
    //                             0x18, 0x10, 0x82, 0x6, 0x2e, 0xfb};

    // uint8_t gnb_rrc_setup_request[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                    0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    //                                    0x3d, 0x2f, 0xc4, 0x40, 0x0, 0x40, 0x11, 0xc,
    //                                    0xea, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    //                                    0x1, 0xca, 0x94, 0x27, 0xf, 0x0, 0x29, 0xfe,
    //                                    0x3c, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
    //                                    0x0, 0x3, 0x2, 0xb1, 0x3b, 0x3, 0x0, 0x1,
    //                                    0x7, 0x2, 0xdc, 0x0, 0x11, 0x1, 0x34, 0x14,
    //                                    0x53, 0xd0, 0x64, 0x93, 0xa6, 0x3f, 0x21, 0x21,
    //                                    0x21, 0x21};

    // uint8_t gnb_uplink_packet[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    //                                0x3d, 0x2d, 0x62, 0x40, 0x0, 0x40, 0x11, 0xf,
    //                                0x4c, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    //                                0x1, 0xe7, 0xa6, 0x27, 0xf, 0x0, 0x29, 0xfe,
    //                                0x3c, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
    //                                0x0, 0x3, 0x2, 0x2e, 0xfb, 0x3, 0x0, 0x1,
    //                                0x7, 0x0, 0xc0, 0x0, 0x11, 0x1, 0x3e, 0x1,
    //                                0x0, 0x39, 0x34, 0x32, 0x3f, 0x21, 0x21, 0x21,
    //                                0x21, 0x21};

    // uint8_t gnb_downlink_packet[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                  0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    //                                  0x47, 0x2d, 0x9a, 0x40, 0x0, 0x40, 0x11, 0xf,
    //                                  0xa, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    //                                  0x1, 0xe7, 0xa6, 0x27, 0xf, 0x0, 0x33, 0xfe,
    //                                  0x46, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
    //                                  0x1, 0x3, 0x2, 0x2e, 0xfb, 0x3, 0x0, 0x1,
    //                                  0x7, 0x0, 0xca, 0x0, 0x1, 0x1, 0x3d, 0x1c,
    //                                  0x3f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                  0x0, 0x0, 0x0, 0x0};

    // uint8_t gnb_uplink_rlc_packet[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                    0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    //                                    0x81, 0x2f, 0xc7, 0x40, 0x0, 0x40, 0x11, 0xc,
    //                                    0xa3, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    //                                    0x1, 0xca, 0x94, 0x27, 0xf, 0x0, 0x6d, 0xfe,
    //                                    0x80, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
    //                                    0x0, 0x3, 0x2, 0xb1, 0x3b, 0x3, 0x0, 0x1,
    //                                    0x7, 0x2, 0xf7, 0x0, 0x9, 0x1, 0x1, 0x23,
    //                                    0xc0, 0x0, 0x0, 0x0, 0x12, 0x0, 0x5, 0xdf,
    //                                    0x80, 0x10, 0x5e, 0x40, 0x3, 0x40, 0x40, 0xbe,
    //                                    0x16, 0x7c, 0x3f, 0xc0, 0x0, 0x0, 0x0, 0x0,
    //                                    0x0, 0x4, 0xcb, 0x80, 0xbc, 0x1c, 0x0, 0x0,
    //                                    0x0, 0x0, 0x0, 0x3d, 0x0, 0x39, 0x3f, 0x35,
    //                                    0x3f, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
    //                                    0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
    //                                    0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
    //                                    0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
    //                                    0x21, 0x21, 0x21, 0x21, 0x21, 0x21};

    // uint8_t gnb_downlink_rlc_packet[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    //                                      0x0, 0x0, 0x0, 0x8, 0x0, 0x45, 0x0, 0x0,
    //                                      0x38, 0x2f, 0xc8, 0x40, 0x0, 0x40, 0x11, 0xc,
    //                                      0xeb, 0x7f, 0x0, 0x0, 0x1, 0x7f, 0x0, 0x0,
    //                                      0x1, 0xca, 0x94, 0x27, 0xf, 0x0, 0x24, 0xfe,
    //                                      0x37, 0x6d, 0x61, 0x63, 0x2d, 0x6e, 0x72, 0x2,
    //                                      0x1, 0x3, 0x2, 0xb1, 0x3b, 0x3, 0x0, 0x1,
    //                                      0x7, 0x2, 0xf7, 0x0, 0x11, 0x1, 0x41, 0x0,
    //                                      0x3, 0x0, 0x1, 0x0, 0x3f};

    // printf("\n------- API DEMO (5G MAC-NR) --------\n");
    // packet_set_protocol("proto:mac-nr-framed");

    // packet_dissect(gnb_rar_packet + 48, sizeof(gnb_rar_packet) - 48);
    // puts(packet_summary());
    // puts("-------------------------------------");

    // packet_dissect(gnb_rrc_setup_request + 48, sizeof(gnb_rrc_setup_request) - 48);
    // puts(packet_summary());
    // // packet_navigate(1, 0, test_navigate_callback);
    // puts("-------------------------------------");

    // packet_dissect(gnb_uplink_packet + 48, sizeof(gnb_uplink_packet) - 48);
    // puts(packet_summary());
    // puts("-------------------------------------");

    // packet_dissect(gnb_downlink_packet + 48, sizeof(gnb_downlink_packet) - 48);
    // puts(packet_summary());
    // puts("-------------------------------------");

    // packet_dissect(gnb_uplink_rlc_packet + 48, sizeof(gnb_uplink_rlc_packet) - 48);
    // puts(packet_summary());
    // puts("-------------------------------------");

    // packet_dissect(gnb_downlink_rlc_packet + 48, sizeof(gnb_downlink_rlc_packet) - 48);
    // puts(packet_summary());
    // puts("-------------------------------------");

    // printf("\nLayers count: %d\n", packet_layers_count());
    // printf("Layers: [%s]\n", packet_dissectors());

    // printf("\nDissector success!\n\n");

    return 0;
}
#endif
