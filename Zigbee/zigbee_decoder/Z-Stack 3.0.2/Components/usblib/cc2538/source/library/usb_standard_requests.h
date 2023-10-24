//*****************************************************************************
//! @file       usb_standard_request.h
//! @brief      Handle USB standard requests.
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

#ifndef __USB_STANDARD_REQUESTS_H__
#define __USB_STANDARD_REQUESTS_H__
/** \addtogroup module_usb_standard_requests  USB Standard Requests (usbsr)
 *
 * \brief This module contains automated functions for processing USB standard requests
 *
 * The processing functions are based on the \ref module_usb_framework, and access to the user-provided
 * USB descriptors through the \ref module_usb_descriptor_parser. All device classes and descriptor
 * combinations are supported, with no need to write or modify library source code. However, as described
 * below, some standard request must be fully or partially implemented by the application.
 *
 * \section section_usbsr_hooks Hooks
 * Standard requests that are not supported by the USB library or that refer to class- or vendor-
 * specified features, are forwarded to the application via function hooks. This includes:
 * - All \c SET_DESCRIPTOR requests (see \ref usbsrHookSetDescriptor())
 * - All \c SYNCH_FRAME requests (see \ref usbsrHookSynchFrame())
 * - \c CLEAR_FEATURE requests that refer to unknown features (see \ref usbsrHookClearFeature())
 * - \c SET_FEATURE requests that refer to unknown features (see \ref usbsrHookSetFeature())
 *
 * These hooks must always be provided, however if the application does not either support the requests,
 * it should just stall endpoint 0. The processing uses the same mechanisms as for class and vendor
 * requests (refer to the \ref module_usb_framework module for detailed description).
 *
 * When the \c GET_STATUS request is received, the \ref usbsrHookModifyGetStatus() hook is always called,
 * so that additional status bits may be added.
 *
 * To have any practical purpose, some "OUT data stage" standard requests need to notify the application. 
 * This is done by passing the event via another function hook,
 * \ref usbsrHookProcessEvent(uint8_t event, uint8_t index). For events related to interfaces and
 * endpoints, the \c index parameter refers to the interface number or the least significant nibble of 
 * the endpoint address. The following events can be generated:
 * - \ref USBSR_EVENT_CONFIGURATION_CHANGING - The device configuration is about to change
 * - \ref USBSR_EVENT_CONFIGURATION_CHANGED -  The device configuration has changed
 * - \ref USBSR_EVENT_INTERFACE_CHANGING - An interface's alternate setting is about to change
 * - \ref USBSR_EVENT_INTERFACE_CHANGED - An interface's alternate setting has changed
 * - \ref USBSR_EVENT_REMOTE_WAKEUP_ENABLED - Remote wakeup has been enabled by the host
 * - \ref USBSR_EVENT_REMOTE_WAKEUP_DISABLED - Remote wakeup has been disabled by the host
 * - \ref USBSR_EVENT_EPIN_STALL_CLEARED - An IN endpoint's stall condition has been cleared by the host
 * - \ref USBSR_EVENT_EPIN_STALL_SET - An IN endpoint has been stalled by the host
 * - \ref USBSR_EVENT_EPOUT_STALL_CLEARED - An OUT endpoint's stall condition has been cleared by the 
 *   host
 * - \ref USBSR_EVENT_EPOUT_STALL_SET - An OUT endpoint has been stalled by the host
 *
 * @{
 */


//-------------------------------------------------------------------------------------------------------
/// \name Standard Request Codes
//@{

//
/// Standard request that returns status for the specified recipient
//
#define USBSR_REQ_GET_STATUS           0x00

//
/// Standard request that clears or disables a specific feature
//
#define USBSR_REQ_CLEAR_FEATURE        0x01

//
/// Standard request that sets or enables a specific feature
//
#define USBSR_REQ_SET_FEATURE          0x03

//
/// Standard request that sets the device address for all future device accesses
//
#define USBSR_REQ_SET_ADDRESS          0x05

//
/// Standard request that returns the specified USB descriptor
//
#define USBSR_REQ_GET_DESCRIPTOR       0x06

//
/// Standard request that may be used to update exitsting descriptors or new descriptors may be added
//
#define USBSR_REQ_SET_DESCRIPTOR       0x07

//
/// Standard request that returns the current device configuration value
//
#define USBSR_REQ_GET_CONFIGURATION    0x08

//
/// Standard request that sets the device configuration
//
#define USBSR_REQ_SET_CONFIGURATION    0x09

//
/// Standard request that returns the selected alternate setting for the specified interface
//
#define USBSR_REQ_GET_INTERFACE        0x0A

//
/// Standard request that selects an alternate setting for the specified interface
//
#define USBSR_REQ_SET_INTERFACE        0x0B

//
/// Standard request that is used to set and then report an endpoint's synchronization frame
//
#define USBSR_REQ_SYNCH_FRAME          0x0C
//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Feature Selectors
//@{

//
/// Endpoint feature: Halt
//
#define USBSR_FEATSEL_ENDPOINT_HALT        0x00

//
/// Device feature: Remote wakeup
//
#define USBSR_FEATSEL_DEVICE_REMOTE_WAKEUP 0x01
//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Event Types
/// Used as event parameter in <tt>usbsrHookProcessEvent(uint8_t event, uint8_t index)</tt>
//@{

//
/// The device configuration is about to change
//
#define USBSR_EVENT_CONFIGURATION_CHANGING  0x01

//
/// The device configuration has changed
//
#define USBSR_EVENT_CONFIGURATION_CHANGED   0x02

//
/// The alternate setting of the given interface about to change (index = "interface index")
//
#define USBSR_EVENT_INTERFACE_CHANGING      0x03

//
/// The alternate setting of the given interface has changed (index = "interface index")
//
#define USBSR_EVENT_INTERFACE_CHANGED       0x04

//
/// Remote wakeup has been enabled by the host
//
#define USBSR_EVENT_REMOTE_WAKEUP_ENABLED   0x05

//
/// Remote wakeup has been disabled by the host
//
#define USBSR_EVENT_REMOTE_WAKEUP_DISABLED  0x06

//
/// The given IN endpoint has been unstalled by the host (index = "endpoint address" & 0x0F)
//
#define USBSR_EVENT_EPIN_STALL_CLEARED      0x07

//
/// The given IN endpoint has been stalled by the host (index = "endpoint address" & 0x0F)
//
#define USBSR_EVENT_EPIN_STALL_SET          0x08

//
/// The given OUT endpoint has been unstalled by the host (index = "endpoint address" & 0x0F)
//
#define USBSR_EVENT_EPOUT_STALL_CLEARED     0x09

//
/// The given OUT endpoint has been stalled by the host (index = "endpoint address" & 0x0F)
//
#define USBSR_EVENT_EPOUT_STALL_SET         0x0A
//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Standard Request Hooks
//@{
/// Refer to the \ref section_setup_handler_usage section for a description on how to process standard
/// requests.

//
/// Hook which is called upon reception of a \c SET_DESCRIPTOR request
//
void usbsrHookSetDescriptor(void);

//
/// Hook which is called upon reception of a \c SYNCH_FRAME request
//
void usbsrHookSynchFrame(void);

//
/// Hook which is called when a \c CLEAR_FEATURE request refers to a an unsupported feature
//
void usbsrHookClearFeature(void);

//
/// Hook which is called when a \c SET_FEATURE request refers to a an unsupported feature
//
void usbsrHookSetFeature(void);

//
/// Hook for modifying a \c GET_STATUS request before the status value is returned to the host
//
uint8_t usbsrHookModifyGetStatus(USB_SETUP_HEADER* pSetupHeader, uint8_t ep0Status, uint16_t* pStatus);

//
/// Hook which is called upon a standard request generated event
//
void usbsrHookProcessEvent(uint8_t event, uint8_t index);
//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Handled Standard Requests
//@{
void usbsrGetStatus(void);
void usbsrClearFeature(void);
void usbsrSetFeature(void);
void usbsrSetAddress(void);
void usbsrGetDescriptor(void);
void usbsrGetConfiguration(void);
void usbsrSetConfiguration(void);
void usbsrGetInterface(void);
void usbsrSetInterface(void);
//@}
//-------------------------------------------------------------------------------------------------------

//@}

#endif // __USB_STANDARD_REQUESTS_H__
