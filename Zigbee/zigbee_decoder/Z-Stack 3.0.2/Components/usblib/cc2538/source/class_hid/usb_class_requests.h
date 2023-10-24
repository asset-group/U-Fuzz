//*****************************************************************************
//! @file       usb_class_request.h
//! @brief      Handle USB class requests for HID class.
//!
//! Revised     $Date: 2013-04-08 04:14:23 -0700 (Mon, 08 Apr 2013) $
//! Revision    $Revision: 9658 $
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

#ifndef USB_CLASS_REQUEST_H
#define USB_CLASS_REQUEST_H

//
// HID Interface indices (as used in USB descriptor) for class request parsing
//
#define KEYBOARD_INDEX          0
#define MOUSE_INDEX             1

//
// Constants specifying HID Class requests (bRequest)
//
#define GET_REPORT              0x01
#define GET_IDLE                0x02
#define GET_PROTOCOL            0x03
#define SET_REPORT              0x09
#define SET_IDLE                0x0A
#define SET_PROTOCOL            0x0B

//
// Report types for use with the GET_/SET_REPORT request
//
#define HID_REP_TYPE_INPUT      1
#define HID_REP_TYPE_OUTPUT     2
#define HID_REP_TYPE_FEATURE    3

//
// Idle rate constants used for GET_/SET_IDLE request
//
#define HID_IDLE_INDEFINITE     0x0000
#define HID_IDLE_NOT_SET        0x00FF      // Used as dummy init value

//
// Protocol constants for use with GET_/SET_PROTOCOL request
//
#define HID_PROTOCOL_BOOT       0
#define HID_PROTOCOL_REPORT     1


//
// Function prototypes
//
void usbcrSetReport(void);
void usbcrSetProtocol(void);
void usbcrSetIdle(void);
void usbcrGetReport(void);
void usbcrGetProtocol(void);
void usbcrGetIdle(void);


#endif // USB_CLASS_REQUEST_H
