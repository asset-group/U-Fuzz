//*****************************************************************************
//! @file       usb_descriptor_parser.h
//! @brief      USB descriptor parser.
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

#ifndef __USB_DESCRIPTOR_PARSER_H__
#define __USB_DESCRIPTOR_PARSER_H__
/** \addtogroup module_usb_descriptor_parser  USB Descriptor Parser (usbdp)
 *
 * \brief This module contains internally used functions for locating USB descriptors.
 *
 * The generic parsing mechanism supports all standard descriptors, including 
 * DEVICE, CONFIGURATION, INTERFACE and  ENDPOINT, but also other types that 
 * use the standard descriptor format:
 * \code
 * typedef __packed struct {
 *     uint8_t bLength;                // Size of this descriptor (in bytes)
 *     uint8_t bDescriptorType;        // Descriptor type
 *     ...
 * } USB_XXXXX_DESCRIPTOR;
 * \endcode
 *
 * An application dependent lookup table must be declared to support retrieval 
 * of:
 * - String descriptors with language ID
 * - Non-standard formatted descriptors, such as the HID class' report descriptors
 * - Standard-formatted class-defined descriptors, such as the \ref USBHID_DESCRIPTOR
 *
 * The lookup table is a null-terminated list of \ref USB_DESCRIPTOR_LUT that 
 * specifies what to return for different value/index parameters of the 
 * GET_DESCRIPTOR request.
 *
 * @{
 */

 
//
// Function prototypes
//
void usbdpInit(void);
const void* usbdpFindNext(uint8_t wantedType, uint8_t haltAtType);
const USB_DEVICE_DESCRIPTOR* usbdpGetDeviceDesc(void);
const USB_CONFIGURATION_DESCRIPTOR* usbdpGetConfigurationDesc(uint8_t cfgValue, 
                                                              uint8_t cfgIndex);
const USB_INTERFACE_DESCRIPTOR* usbdpGetInterfaceDesc(uint8_t cfgValue, 
                                                      uint8_t intNumber, 
                                                      uint8_t altSetting);
const USB_DESCRIPTOR_LUT* usbdpGetDescByLut(uint16_t value, uint16_t index);


#endif // __USB_DESCRIPTOR_PARSER_H__
//@}
