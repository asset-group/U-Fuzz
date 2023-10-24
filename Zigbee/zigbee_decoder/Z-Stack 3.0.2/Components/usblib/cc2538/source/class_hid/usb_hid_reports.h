//*****************************************************************************
//! @file       usb_hid_reports.h
//! @brief      Definitions and prototypes for HID reports.
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

#ifndef USBHIDREPORTS_H
#define USBHIDREPORTS_H


#include <stdint.h>

typedef struct 
{
    uint8_t ledStatus;
} KEYBOARD_OUT_REPORT;

typedef struct 
{
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t pKeyCodes[6];
} KEYBOARD_IN_REPORT;

typedef struct 
{
    uint8_t buttons;
    int8_t dX;
    int8_t dY;
    int8_t dZ;
} MOUSE_IN_REPORT;


typedef struct 
{
    KEYBOARD_OUT_REPORT keyboardOutReport;
    KEYBOARD_IN_REPORT keyboardInReport;
    MOUSE_IN_REPORT mouseInReport;
    uint8_t keyboardProtocol;
    uint8_t mouseProtocol;
    uint16_t keyboardIdleRate;
    uint16_t mouseIdleRate;
} HID_DATA;


extern HID_DATA hidData;


uint8_t hidSendKeyboardInReport(void);
uint8_t hidSendMouseInReport(void);
void hidUpdateKeyboardInReport(KEYBOARD_IN_REPORT *pNewReport);
void hidUpdateMouseInReport(MOUSE_IN_REPORT *pNewReport);
void hidShowKeyboardLeds(void);


#endif // USBHIDREPORTS_H
