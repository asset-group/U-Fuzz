
#ifndef __GENERIC_H__
#define __GENERIC_H__

#include "config.h"

#include <stdio.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <epan/packet.h>

#ifdef __cplusplus
}
#endif /* __cplusplus */



#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    
#endif
#endif


#ifdef __cplusplus
/******************************************************************************
 * dissect
 *****************************************************************************/

proto_tree  * cpp_dissect_generic_add_tree(const int        proto_idx,
                                           proto_item     * proto_item_void);


#define K_WHOLE_WSGD_FIELD_IDX    0
#define K_ANY_WSGD_FIELD_IDX      0
#define K_ERROR_WSGD_FIELD_IDX    1

#endif

/******************************************************************************
 * register
 *****************************************************************************/
EXTERN_C
void    cpp_proto_register_generic(void);

//*****************************************************************************
// handoff
//*****************************************************************************
EXTERN_C
void    cpp_proto_reg_handoff_generic(void);


#endif
