//*****************************************************************************
//! @file       usb_suspend.h
//! @brief      Handle the USB suspend state.
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

#ifndef __USB_SUSPEND_H__
#define __USB_SUSPEND_H__
/** \addtogroup module_usb_suspend USB Suspend (usbsusp)
 *
 * \brief This module contains the functionality for USB suspend, USB resume and USB remote wakeup.
 *
 * \section usb_suspend_resume USB Suspend
 * USB devices must support the suspended state, which is the standardized way for USB devices to reduce 
 * current consumption while the USB host is sleeping. If there is no activity on the USB bus for a 
 * period longer than 3 ms, the USB device is said to be in the suspended state, and the USB library 
 * generates a \ref USBIRQ_EVENT_SUSPEND event. The application must then enter low-power mode within 
 * 7 ms (i.e. 10 ms after the last activity on the bus), where it draws no more than 2.5 mA averaged over 
 * 1 second.
 *
 * Exit from the suspended state can either be host- or device-initiated:
 * - Resume (host-initiated): The USB device exits the suspended state upon activity on the bus. This
 *   activity can be resume signaling, reset signaling or USB packets.
 * - Remote wakeup (device-initiated): The USB device generates remote wakeup signaling on the bus to 
 *   alert the USB host. Remote wakeup is only allowed if declared in the USB descriptor
 *   (\ref USB_CONFIGURATION_DESCRIPTOR.bmAttributes) and the host has enabled it with a SET_FEATURE
 *   request.
 *
 * The USB firmware library supports the USB suspend and resume functionality through a simple interface:
 * - The high-speed XOSC is never turned off anywhere in the application.
 * - In the call to \ref usbirqInit(), add \ref USBIRQ_EVENT_SUSPEND and \ref USBIRQ_EVENT_RESUME
 *   (optional) to the interrupt mask.
 * - The \ref USBIRQ_EVENT_SUSPEND must be processed in main context. If processed in interrupt context,
 *   including the \ref usbirqHookProcessEvents() callback, the library may deadlock and prevent exit 
 *   from suspend mode.
 * - Add the code shown below to handle suspend events. Make sure that this code is visited at least
 *   every 7 ms. If the worst-case path through the main loop uses more than 7 ms, the code block can be
 *   inserted in multiple places in the loop until the requirement is met.
 *
 * \code
 * // Process USB suspend events
 * if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_SUSPEND) {
 *
 *     // Clear the suspend event
 *     USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_SUSPEND);
 *
 *     // This function call will take the system into low-power mode with full register and RAM 
 *     // retention. It will not return until resume signaling or equivalent has been detected on the 
 *     // bus, or the remote wake-up function has been used. Other interrupts (for instance from 
 *     // I/O ports or the sleep timer) can be used during this period. When returning from these 
 *     // interrupts, the \ref usbsuspEnter() function (running here) will put the system back into the
 *     // low-power mode.
 *     usbsuspEnter();
 *
 *     // At this point the event handler is up and running again. Clear the resume event.
 *     USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_RESUME);
 *
 * }
 * \endcode
 *
 * - If the \ref usbsuspEnter() function actually enters suspend mode - it might not if resume signaling 
 *   already has been detected on the bus - it will generate callbacks to the application at entry and 
 *   exit. At entry, the \c remoteWakeupAllowed parameter to \ref usbsuspHookEnteringSuspend() indicates
 *   whether the device is allowed to perform remote wakeup:
 * \code
 * void usbsuspHookEnteringSuspend(bool remoteWakeupAllowed) {
 *     ... Prepare entry into suspended mode by disabling application functionality entirely, or by
 *         starting duty-cycled operation with low average current consumption ...
 *
 *     // After this callback, the application must avoid accessing USB registers (e.g. loading keyboard 
 *     // HID input reports into an endpoint FIFO)
 * }
 *
 * void usbsuspHookExitingSuspend(void) {
 *     ... If a USBIRQ_EVENT_RESET event will affect the code that follows (before the event is processed
 *         elsewhere), then make sure to handle it here also ...
 *
 *     ... Re-enable application functionality for normal active operation ...
 *     
 *     // During and after this callback, the application can continue accessing USB registers
 * }
 * \endcode
 *
 * \section usb_remote_wakeup USB Remote Wakeup
 * Remote wakeup should be used when the USB device wants to initiate the resume process and wake up the
 * host. In a radio application this may happen when a particular radio packet is received, for instance
 * from a wireless keyboard or mouse.
 *
 * USB remote wakeup can only be performed if the host has given the device the privilege to do so. The
 * privilege to perform remote wakeup is requested by setting bit 5 in the \c bmAttributes field in
 * the \ref USB_CONFIGURATION_DESCRIPTOR. The host controls the privilege with \c SET_FEATURE and 
 * \c CLEAR_FEATURE requests, which is communicated through a \ref USBSR_EVENT_REMOTE_WAKEUP_ENABLED or
 * \ref USBSR_EVENT_REMOTE_WAKEUP_DISABLED event, respectively.
 *
 * The USB library handles the remote wakeup sequence automatically. Do the following to incorporate it
 * into the application:
 * - Implement suspend and resume as described above.
 * - In the USB descriptor, set bit 5 in bmAttributes in the configuration descriptor.
 * - While the USB MCU is in USB suspend mode, remote wakeup can be performed from interrupt context
 *   (e.g. the sleep timer interrupt) by calling \ref usbsuspDoRemoteWakeup(). This function will return 
 *   TRUE if successful or FALSE if remote wakeup is not permitted (by the host).
 *
 * @{
 */


//
// Suspend enter/exit hooks
//
void usbsuspHookEnteringSuspend(bool remoteWakeupAllowed);
void usbsuspHookExitingSuspend(void);


//
// Function prototypes
//
void usbsuspEnter(void);
uint8_t usbsuspDoRemoteWakeup(void);


//@}

#endif // __USB_SUSPEND_H__
