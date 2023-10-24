//*****************************************************************************
//! @file       usb_firmware_library_headers.h
//! @brief      Common inclusion of all USB library headers.
//!
//! Revised     $Date: 2013-04-02 05:52:48 -0700 (Tue, 02 Apr 2013) $
//! Revision    $Revision: 9573 $
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/

#ifndef __USB_FIRMWARE_LIBRARY_HEADERS_H__
#define __USB_FIRMWARE_LIBRARY_HEADERS_H__


#ifdef EXTERN
   #undef EXTERN
#endif
#ifdef USBFRAMEWORK_C
   #define EXTERN ///< Definition used only for usb_framework.c
#else
   #define EXTERN extern ///< Definition used in other source files to declare external
#endif


#ifndef st
    #define st( x )     do { x } while (__LINE__ == -1)
#endif


// Standard type and constant definitions
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Hardware register definitions
#include "hw_usb.h"
#include "hw_usb_aliases.h"
#include "hw_cctest.h"
#include "hw_gpio.h"

// USB device firmware library
#include "usb.h"
#include "usb_descriptor_defs.h"
#include "usb_descriptor_parser.h"
#include "usb_interrupt.h"
#include "usb_framework.h"
#include "usb_standard_requests.h"
#include "usb_suspend.h"

// Device driver library files
#include "sys_ctrl.h"
#include "interrupt.h"
#include "cpu.h"


#endif //__USB_FIRMWARE_LIBRARY_HEADERS_H__

