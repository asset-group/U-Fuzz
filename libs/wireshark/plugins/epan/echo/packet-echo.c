#include "config.h"
#include "echo.h"

#include <epan/packet.h>

#define ECHO_PORT 3001

static int proto_echo = -1;

static gint ett_echo = -1;

static int dissect_echo(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
   ((void) data); // suppress compile warning.

   col_set_str(pinfo->cinfo, COL_PROTOCOL, "ECHO");
   col_clear(pinfo->cinfo,COL_INFO);

   if (tree) { /* we are being asked for details */
      proto_item *ti;
      EchoPacket *msg;

      ti = proto_tree_add_item(tree, proto_echo, tvb, 0, -1, FALSE);

      tree = proto_item_add_subtree(ti, ett_echo);

      msg = (EchoPacket *) wmem_alloc(wmem_packet_scope(), sizeof(EchoPacket));

      /* call the TSN.1 Compiler generated dissect function */
      EchoPacket_dissect(msg, proto_echo, tvb, 0, pinfo, tree);
   }

   return tvb_captured_length(tvb);
}

void proto_register_echo(void)
{
   /* Setup protocol subtree array */
   static int *ett[] = { &ett_echo };

   /* We use the name "echo2" here because there is already a built-in
      protocol called "echo". */
   proto_echo = proto_register_protocol
      (
         "TSNC Echo Protocol", /* name */
         "TSNC_ECHO",         /* short name */
         "tsnc_echo"          /* abbrev */
      );

   /* call the TSN.1 Compiler generated register function */
   EchoPacket_register(proto_echo);
   proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_echo(void)
{
   static dissector_handle_t echo_handle;

   echo_handle = create_dissector_handle(dissect_echo, proto_echo);
   dissector_add_uint("udp.port", ECHO_PORT, echo_handle);
}
