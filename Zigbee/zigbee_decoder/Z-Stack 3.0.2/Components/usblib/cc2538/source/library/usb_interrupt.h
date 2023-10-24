//*****************************************************************************
//! @file       usb_interrupt.h
//! @brief      USB library interrupt initialisation and ISR.
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

#ifndef __USB_INTERRUPT_H__
#define __USB_INTERRUPT_H__
/** \addtogroup module_usb_interrupt  USB Interrupt (usbirq)
 *
 * \brief This module implements the USB interrupt handlers, which converts USB interrupts to events and 
 *        handles resuming after USB suspend mode
 *
 * This module contains two interrupt service routines:
 * - The USB function ISR, which is executed upon relevant bus traffic and signaling
 * - The USB wakeup ISR, used to detect resume signaling while no clock is provided to the USB function
 *
 * The USB peripheral unit has three interrupt flag registers, \c USB_CIF, \c USB_IIF and \c USB_OIF, 
 * which are cleared upon read access. The USB function ISR converts the interrupt flags into events 
 * stored in a 16-bit word, allowing the application to process high-priority events in interrupt 
 * context, and low-priority events in the main context.
 *
 * \section section_usbirq_initialization Initialization
 * The \ref module_usb_interrupt module must be initialized by calling \ref usbirqInit(). This must be 
 * done before initializing the \ref module_usb_framework module, and before enabling the USB D+ data 
 * line pull-up resistor. The \c irqMask parameter of usbirqInit() specifies all \c USBIRQ_EVENT flags
 * that will be handled either in interrupt or main context. Note that event reporting is not always 
 * necessary. For instance, there is usually no reason to enable \c USBIRQ_EVENT_EPxIN and 
 * \c USBIRQ_EVENT_EPxOUT events when handling regular bulk or interrupt transfers in the main loop. In 
 * these cases it is simpler and more efficient to just check the arming condition of the used endpoint.
 * 
 * The \ref USBIRQ_EVENT_RESET and \ref USBIRQ_EVENT_SETUP events must always be included in the 
 * \c irqMask. For bus-powered USB devices the \ref USBIRQ_EVENT_SUSPEND event also mandatory to pass 
 * product certification.
 *
 * The following code example enables the setup, reset, suspend and resume events:
 * \code
 * void main(void) {
 *
 *     ... Initialize the crystal oscillator and USB framework first ...
 *
 *     // Initialize the USB Interrupt module
 *     usbirqInit(USBIRQ_EVENT_RESET | USBIRQ_EVENT_SUSPEND | USBIRQ_EVENT_RESUME | USBIRQ_EVENT_SETUP);
 *
 *     // Main loop
 *     while (1) {
 *         ...
 *     }
 * \endcode
 *
 * \section section_usbirq_event_processing Event Processing
 * When processing events, check and clear the event flags and process in the following manner (this 
 * example illustrates the processing of \ref USBIRQ_EVENT_RESET events):
 * \code
 * // Let the framework handle reset events
 * if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_RESET) {
 *      USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_RESET);
 *      usbfwResetHandler();
 * }
 * \endcode
 *
 * \section Hooks
 * The following hook is called from the USB function interrupt, and allows for event processing in 
 * interrupt context:
 * \code
 * void usbirqHookProcessEvents(void) {
 *     // Process high-priority events here, or simply return if handled in main context
 * }
 * \endcode
 *
 * @{
 */


//-------------------------------------------------------------------------------------------------------
//
/// USBIRQ internal module data
//
typedef struct {
    volatile uint16_t eventMask;      ///< Bit mask containing all pending events (see the \c USBIRQ_EVENT definitions)
    uint16_t          irqMask;        ///< USB interrupts to be enabled
    volatile uint8_t  inSuspend;      ///< Is currently in suspend?
    volatile uint8_t  waitForPm1Exit; ///< Waiting for PM1 exit due to USB data line activity?
} USBIRQ_DATA;

//
/// USBIRQ internal module data
//
EXTERN USBIRQ_DATA usbirqData;
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name USB Interrupt Events
//@{

//
/// Suspend signaling detected on the USB bus
//
#define USBIRQ_EVENT_SUSPEND         0x0001

//
/// Resume signaling detected on the USB bus
//
#define USBIRQ_EVENT_RESUME          0x0002

//
/// Reset signaling detected on the USB bus (call \ref usbfwResetHandler() for processing)
//
#define USBIRQ_EVENT_RESET           0x0004

//
/// Start of frame token received (synthesized by hardware when the next SOF token is expected, so that missing or corrupted tokens have no effect)
//
#define USBIRQ_EVENT_START_OF_FRAME  0x0008

//
/// Endpoint 0 IN/OUT setup/data transfer complete / stall sent / premature completion (call \ref usbfwSetupHandler() for processing)
//
#define USBIRQ_EVENT_SETUP           0x0010

//
/// Endpoint 1 IN data successfully transmitted to host (FIFO disarmed) / FIFO flushed / stall sent
//
#define USBIRQ_EVENT_EP1IN           0x0020

//
/// Endpoint 2 IN data successfully transmitted to host (FIFO disarmed) / FIFO flushed / stall sent
//
#define USBIRQ_EVENT_EP2IN           0x0040

//
/// Endpoint 3 IN data successfully transmitted to host (FIFO disarmed) / FIFO flushed / stall sent
//
#define USBIRQ_EVENT_EP3IN           0x0080

//
/// Endpoint 4 IN data successfully transmitted to host (FIFO disarmed) / FIFO flushed / stall sent
//
#define USBIRQ_EVENT_EP4IN           0x0100

//
/// Endpoint 5 IN data successfully transmitted to host (FIFO disarmed) / FIFO flushed / stall sent
//
#define USBIRQ_EVENT_EP5IN           0x0200

//
/// Endpoint 1 OUT data received from host (FIFO disarmed) / stall sent
//
#define USBIRQ_EVENT_EP1OUT          0x0400

//
/// Endpoint 2 OUT data received from host (FIFO disarmed) / stall sent
//
#define USBIRQ_EVENT_EP2OUT          0x0800

//
/// Endpoint 3 OUT data received from host (FIFO disarmed) / stall sent
//
#define USBIRQ_EVENT_EP3OUT          0x1000

//
/// Endpoint 4 OUT data received from host (FIFO disarmed) / stall sent
//
#define USBIRQ_EVENT_EP4OUT          0x2000

//
/// Endpoint 5 OUT data received from host (FIFO disarmed) / stall sent
//
#define USBIRQ_EVENT_EP5OUT          0x4000
//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Interrupt Mask Access Macros
//@{

//
/// Clears one or more events (use one or more \c USBIRQ_EVENT bits OR'ed together)
//
#define USBIRQ_CLEAR_EVENTS(mask) \
    st( \
        bool intDisabled = IntMasterDisable(); \
        usbirqData.eventMask &= ~(mask); \
        if (!intDisabled) IntMasterEnable(); \
    )

//
/// Get the bit mask containing all pending events
//
#define USBIRQ_GET_EVENT_MASK()      (usbirqData.eventMask)

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Interrupt Event Hooks
/// Important: Do not enter PM1 while from the usbirqHookProcessEvents() callback
//@{

//
/// Called upon all USB interrupts for high-priority event processing
//
void usbirqHookProcessEvents(void);

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Function prototypes
void usbirqInit(uint16_t irqMask);
//-------------------------------------------------------------------------------------------------------


//@}

#endif // __USB_INTERRUPT_H__
