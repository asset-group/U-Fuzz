//*****************************************************************************
//! @file       app_usb_descriptor.h
//! @brief      USB descriptor for HID class.
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

#ifndef __APP_USB_DESCRIPTOR_H__
#define __APP_USB_DESCRIPTOR_H__

#include "usb_descriptor_defs.h"

//
// Defines the USB string descriptor struct types (index, number of unicode characters)
//
TYPEDEF_USB_STRING_DESCRIPTOR(0, 1);
TYPEDEF_USB_STRING_DESCRIPTOR(1, 17);
TYPEDEF_USB_STRING_DESCRIPTOR(2, 14);
TYPEDEF_USB_STRING_DESCRIPTOR(3, 20);

//
// Defines the collection of static string descriptors (to ease size calculations)
//
typedef __packed struct 
{
    USB_STRING_0_DESCRIPTOR langIds;
    USB_STRING_1_DESCRIPTOR manufacturer;
    USB_STRING_2_DESCRIPTOR product;
} USB_STRING_DESCRIPTORS;

//
// Defines the static USB descriptor set
//
typedef __packed struct 
{
    USB_DEVICE_DESCRIPTOR device;
    USB_CONFIGURATION_DESCRIPTOR configuration0;
        USB_INTERFACE_DESCRIPTOR interface0;
            USBHID_DESCRIPTOR hid0;
            USB_ENDPOINT_DESCRIPTOR endpoint0;
        USB_INTERFACE_DESCRIPTOR interface1;
            USBHID_DESCRIPTOR hid1;
            USB_ENDPOINT_DESCRIPTOR endpoint1;
    USB_STRING_DESCRIPTORS strings;
} USB_DESCRIPTOR;

extern const USB_DESCRIPTOR usbDescriptor;


//
// Calculates the size of the configuration descriptor as "total - (device + strings)"
//
#define SIZEOF_CONFIGURATION0_DESC      (sizeof(USB_DESCRIPTOR) - (sizeof(USB_DEVICE_DESCRIPTOR) + sizeof(USB_STRING_DESCRIPTORS)))


#endif // __APP_USB_DESCRIPTOR_H__
