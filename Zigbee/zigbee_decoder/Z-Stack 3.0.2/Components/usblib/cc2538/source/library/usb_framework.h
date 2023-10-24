//*****************************************************************************
//! @file       usb_framework.h
//! @brief      USB library common functionality.
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

#ifndef __USB_FRAMEWORK_H__
#define __USB_FRAMEWORK_H__

#ifndef USB_SETUP_MAX_NUMBER_OF_INTERFACES
#warning usb_framework.h: Identifier USB_SETUP_MAX_NUMBER_OF_INTERFACES is \
undefined. Using default value USB_SETUP_MAX_NUMBER_OF_INTERFACES = 5
#define USB_SETUP_MAX_NUMBER_OF_INTERFACES      5
#endif


/** \addtogroup module_usb_framework USB Framework (usbfw)
 *
 * \brief This module contains USB status information, functions for initialization, USB device reset
 * handling, and most importantly, the framework for transfers on endpoint 0, FIFO access, and endpoint
 * control.
 *
 * \section section_init Framework Initialization
 * To enable USB operation, the must complete the following steps in the indicated order:
 * - Ensure that the crystal oscillator is running at the correct speed for USB operation
 * - Call \ref usbfwInit() to initialize the framework state and enable the USB peripheral unit
 * - Call \ref usbirqInit() to initialize USB interrupt handling and configure USB event generation
 * - Indicate that the USB device is present on the bus by enabling the 1.5 kOhm pull-up on the USB D+
 *   data line (unless always enabled in the PCB layout)
 *
 * When completing the last step, the USB host becomes aware of the device, and starts the enumeration
 * process. The enumeration process is handled automatically by the framework, relying only on the
 * user-written USB descriptor set and associated data structures.
 *
 * \section section_endpoint_0_transfers Endpoint 0 Transfers
 * The USB interface uses endpoint 0 to perform setup requests of standard, vendor and class types.
 * Such transfers consist of three stages:
 * - A setup stage, where an 8-byte \ref USB_SETUP_HEADER is transferred to the device
 * - An IN/OUT data stage, if the length field of the \ref USB_SETUP_HEADER structure is non-zero
 * - A handshake stage, where the application can stall the endpoint to indicate error conditions
 *
 * The setup handler, \ref usbfwSetupHandler(), takes care of the low-level parts of these transfers,
 * including the IN/OUT buffering during the data stage (when there is one). The transfers fall into two
 * categories:
 * - Most standard requests are processed internally, with little or no intervention from the user
 *   application. This is done by the \ref module_usb_standard_requests module.
 * - Vendor and class requests are application specific and must always be processed by the application.
 *   Whenever such a request is received, the following hooks will be called between the stages:
 *     - \ref usbcrHookProcessOut(): Class requests with OUT data stage
 *     - \ref usbcrHookProcessIn(): Class requests with IN data stage
 *     - \ref usbvrHookProcessOut(): Vendor requests with OUT data stage
 *     - \ref usbvrHookProcessIn(): Vendor requests with IN data stage
 *
 * \section section_setup_handler_usage Setup Handler Usage
 * This section describes what is required to make the vendor and class request hooks work. This
 * information also applies to the standard requests that needs application processing
 * (\ref usbsrHookSetDescriptor(), \ref usbsrHookSynchFrame(), and some cases of
 * \ref usbsrHookSetFeature() and \ref usbsrHookClearFeature()).
 *
 * The transactions are made using a clean and efficient interface, consisting of two data structures and
 * the endpoint 0 status byte:
 * - The endpoint status is initially \ref EP_IDLE, which means that the setup handler is ready for a new
 *   setup stage (a new request). Upon an incoming request, the processing hook is called, and the
 *   \ref usbSetupHeader structure contains the 8 bytes received during the setup stage. At this point
 *   there are four different outcomes:
 *     - If the request is unknown or contains errors, the endpoint should be stalled. This is done by
 *       setting the endpoint status to \ref EP_STALL.
 *     - If there is no data stage (the length field is zero), the endpoint status should just remain in
 *       it's current state, \ref EP_IDLE.
 *     - If the request has an IN data stage, the \ref usbSetupData structure must be prepared. This
 *       includes setting a pointer to where IN data should be taken from, and the number of bytes to be
 *       transferred (usually the same number as indicated by the length field, but it can also be a
 *       lower number). The endpoint state is then changed to \ref EP_TX.
 *     - If the request has an OUT data stage, the \ref usbSetupData structure must be prepared. This
 *       includes setting a pointer to where OUT data should be stored, and the number of bytes to be
 *       transferred (always the same number as indicated by the length field). The endpoint state is
 *       then changed to \ref EP_RX.
 * - When the data stage is complete, the processing hook function will be called a second time to notify
 *   the application. Under normal conditions the endpoint status will be either \ref EP_TX or
 *   \ref EP_RX, and does not need to be changed any further (as this is done automatically upon return).
 *   If the endpoint status is \ref EP_CANCEL, it means that the USB host cancelled the setup transfer.
 *
 * The following code examples illustrate practical usage (more code is available in example projects):
 *
 * \par Example 1: Endpoint 0 Requests With OUT Data stage
 *
 * \code
 * uint8 pLcdBuffer[358];
 *
 * void usbvrHookProcessOut(void) {
 *
 *     // When this vendor request is received, we should either update a part of pLcdBuffer[] or leave
 *     // it as it is, and then refresh the LCD display. The index field of the setup header contains the
 *     // index of the first character to be updated, and the length field how many characters to update.
 *     if (usbSetupHeader.request == VR_LCD_UPDATE) {
 *
 *         // First the endpoint status is EP_IDLE... (we have just received the Setup packet)
 *         if (usbfwData.ep0Status == EP_IDLE) {
 *
 *             // There is no new data -> Just refresh the display
 *             if (usbSetupHeader.length == 0) {
 *                 lcdRefresh();
 *                 // There is no change to the endpoint status in this case
 *
 *             // The PC wants to send data that will be stored outside pLcdBuffer -> stall the endpoint!
 *             } else if ((usbSetupHeader.length > sizeof(pLcdBuffer) ||
 *                        (usbSetupHeader.index >= sizeof(pLcdBuffer) ||
 *                        ((usbSetupHeader.index + usbSetupHeader.length) > sizeof(pLcdBuffer)) {
 *                 usbfwData.ep0Status = EP_STALL;
 *
 *             // Prepare for OUT data stage, setup the data buffer to receive the LCD data
 *             } else {
 *                 usbSetupData.pBuffer = &pLcdBuffer[usbSetupHeader.index];
 *                 usbSetupData.bytesLeft = usbSetupHeader.length;
 *                 usbfwData.ep0Status = EP_RX;
 *             }
 *
 *         // Then the endpoint status is EP_RX (remember: we did that here when setting up the buffer)
 *         } else if (usbfwData.ep0Status == EP_RX) {
 *             // usbfwSetupHandler() has now updated pLcdBuffer, so all we need to do is refresh the LCD
 *             lcdRefresh();
 *             // usbfwData.ep0Status is automatically reset to EP_IDLE when returning to usbfwSetupHandler()
 *         }
 *
 *     // Unknown vendor request?
 *     } else {
 *         usbfwData.ep0Status = EP_STALL;
 *     }
 * }
 * \endcode
 *
 * \par Example 2: Endpoint 0 Requests With IN Data stage
 *
 * \code
 * uint8 keyBufferPos;
 * bool blockKeyboard;
 * char pKeyBuffer[150];
 *
 * void usbvrProcessIn(void) {
 *
 *     // When this vendor request is received, we should send all registered key-strokes, and reset the
 *     // position counter. New keystrokes are blocked during the transfer to avoid overwriting the buffer
 *     // before it has been sent to the host.
 *     if (usbSetupHeader.request == VR_GET_KEYS) {
 *
 *         // First the endpoint status is EP_IDLE...
 *         if (usbfwData.ep0Status == EP_IDLE) {
 *
 *             // Make sure that we do not send more than the PC asked for
 *             if (usbSetupHeader.length < keyBufferPos) {
 *                 usbfwData.ep0Status = EP_STALL;
 *
 *             // Otherwise...
 *             } else {
 *                 // Block for new keystrokes
 *                 blockKeyboard = TRUE;
 *
 *                 // Setup the buffer
 *                 usbSetupData.pBuffer = pKeyBuffer;
 *                 usbSetupData.bytesLeft = keyBufferPos;
 *                 usbfwData.ep0Status = EP_TX;
 *
 *                 // Reset the position counter
 *                 keyBufferPos = 0;
 *             }
 *
 *         // Then the endpoint status is EP_TX (remember: we did that here when setting up the buffer)
 *         } else if (usbfwData.ep0Status == EP_TX) {
 *
 *             // pKeyBuffer has now been sent to the host, so new keystrokes can safely be registered
 *             blockKeyboard = FALSE;
 *
 *             // usbfwData.ep0Status is automatically reset to EP_IDLE when returning to usbfwSetupHandler()
 *         }
 *
 *     // Unknown request?
 *     } else {
 *        usbfwData.ep0Status = EP_STALL;
 *     }
 * }
 * \endcode
 *
 * If automated data transfer is not desired, the application should set \c usbfwData.ep0Status to
 * either \ref EP_MANUAL_RX or \ref EP_MANUAL_TX instead of \ref EP_RX or \ref EP_TX, respectively. Until
 * the transfer is completed, the processing hook function (e.g. \ref usbvrHookProcessIn()) will be
 * called at every endpoint 0 interrupt.
 * @{
 */


//-------------------------------------------------------------------------------------------------------
/// \name Module Data
//@{

//
/// Endpoint status (used with USBFW_DATA.ep0Status / pEpInStatus[] / pEpOutStatus[])
//
typedef enum {
    EP_IDLE        = 0x00,  ///< The endpoint is idle, or a setup token has been received
    EP_TX          = 0x01,  ///< Setup IN data is transmitted automatically by the framework
    EP_RX          = 0x02,  ///< Setup OUT data is received automatically by the framework
    EP_HALT        = 0x03,  ///< The endpoint is halted (returns stalls to the host)
    EP_STALL       = 0x04,  ///< Send procedural stall in the next status stage
    EP_MANUAL_TX   = 0x05,  ///< Setup IN data is transmitted manually by the user application
    EP_MANUAL_RX   = 0x06,  ///< Setup OUT data is received manually by the user application
    EP_CANCEL      = 0x07,  ///< The current transfer was cancelled by the host
    EP_ADDRESS     = 0x08   ///< Waiting for status stage of SET_ADDRESS to complete
} EP_STATUS;

//
/// Device state (used with USB_INFO.usbState)
//
typedef enum {
    DEV_ATTACHED   = 0x00,  ///< Device attached (invisible state)
    DEV_POWERED    = 0x01,  ///< Device powered (invisible state)
    DEV_DEFAULT    = 0x02,  ///< Default state (the \c USBADDR register is 0)
    DEV_ADDRESS    = 0x03,  ///< Addressed state (the \c USBADDR register has been set)
    DEV_CONFIGURED = 0x04,  ///< Configured state (\c usbfwData.configurationValue != 0)
    DEV_SUSPENDED  = 0x05   ///< Suspended state (never set)
} USB_STATE;

//
/// USBFW internal module data
//
typedef __packed struct {
    uint8_t usbState;           ///< USB device state
    uint8_t ep0Status;          ///< Endpoint 0 status
    uint8_t pEpInStatus[5];     ///< Endpoint 1-5 IN status
    uint8_t pEpOutStatus[5];    ///< Endpoint 1-5 OUT status
    uint8_t configurationValue; ///< Current configuration value
    uint8_t pAlternateSetting[USB_SETUP_MAX_NUMBER_OF_INTERFACES];  ///< Current alternate settings
    uint8_t remoteWakeup;       ///< Remote wakeup allowed
    uint8_t selfPowered;        ///< Is currently self-powered?
} USBFW_DATA;

/// USBFW internal module data
EXTERN USBFW_DATA usbfwData;

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Setup Handler Data
//@{

//
/// Setup header (contains the 8 bytes received during the setup stage)
//
typedef __packed struct {
    uint8_t  requestType;     ///< Request type (direction, type and recipient, see the \c RT_ definitions)
    uint8_t  request;         ///< Request ID
    __packed union {
        uint16_t value;       ///< Value field (little-endian)
        __packed struct {
            uint8_t valueLsb; ///< LSB of value field
            uint8_t valueMsb; ///< MSB of value field
        };
    };
    __packed union {
        uint16_t index;       ///< Value field (little-endian)
        __packed struct {
            uint8_t indexLsb; ///< LSB of value field
            uint8_t indexMsb; ///< MSB of value field
        };
    };
    uint16_t length;          ///< Length of data stage (little-endian)
} USB_SETUP_HEADER;

//
/// Setup header
//
EXTERN USB_SETUP_HEADER usbSetupHeader;

//
/// Setup handler data stage configuration
//
typedef __packed struct {
    void*    pBuffer;     ///< Pointer to where IN/OUT data should be taken from/received
    uint16_t bytesLeft;   ///< The number of bytes to transfer
} USB_SETUP_DATA;

//
/// Setup handler data stage configuration
//
EXTERN USB_SETUP_DATA usbSetupData;

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Request Type Fields
//@{

//
// Field masks
//
#define RT_MASK_DIR       0x80  ///< Request direction bit mask
#define RT_MASK_TYPE      0x60  ///< Request type bit mask
#define RT_MASK_RECIP     0x1F  ///< Request recipient bit mask

//
// Direction field
//
#define RT_DIR_IN         0x80  ///< IN Request
#define RT_DIR_OUT        0x00  ///< OUT Request

//
// Type field
//
#define RT_TYPE_STD       0x00  ///< Standard Request
#define RT_TYPE_CLASS     0x20  ///< Class Request
#define RT_TYPE_VEND      0x40  ///< Vendor Request

//
// Recipient field
//
#define RT_RECIP_DEV      0x00  ///< Device Request
#define RT_RECIP_IF       0x01  ///< Interface Request
#define RT_RECIP_EP       0x02  ///< Endpoint Request
#define RT_RECIP_OTHER    0x03  ///< Other Request

//
// Type + direction
//
#define RT_STD_OUT        (RT_TYPE_STD | RT_DIR_OUT)    ///< Standard request, direction is OUT
#define RT_STD_IN         (RT_TYPE_STD | RT_DIR_IN)     ///< Standard request, direction is IN
#define RT_VEND_OUT       (RT_TYPE_VEND | RT_DIR_OUT)   ///< Vendor request, direction is OUT
#define RT_VEND_IN        (RT_TYPE_VEND | RT_DIR_IN)    ///< Vendor request, direction is IN
#define RT_CLASS_OUT      (RT_TYPE_CLASS | RT_DIR_OUT)  ///< Class request, direction is OUT
#define RT_CLASS_IN       (RT_TYPE_CLASS | RT_DIR_IN)   ///< Class request, direction is IN

//
// Direction + recepient
//
#define RT_OUT_DEVICE     (RT_DIR_OUT | RT_RECIP_DEV)   ///< Request made to device, direction is OUT
#define RT_IN_DEVICE      (RT_DIR_IN | RT_RECIP_DEV)    ///< Request made to device, direction is IN
#define RT_OUT_INTERFACE  (RT_DIR_OUT | RT_RECIP_IF)    ///< Request made to interface, direction is OUT
#define RT_IN_INTERFACE   (RT_DIR_IN | RT_RECIP_IF)     ///< Request made to interface, direction is IN
#define RT_OUT_ENDPOINT   (RT_DIR_OUT | RT_RECIP_EP)    ///< Request made to endpoint, direction is OUT
#define RT_IN_ENDPOINT    (RT_DIR_IN | RT_RECIP_EP)     ///< Request made to endpoint, direction is IN
//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Vendor and Class Request Hooks
/// Unused hooks must stall endpoint 0.
//@{

//
/// Hook which is called upon reception of a class request with OUT data stage
//
void usbcrHookProcessOut(void);

//
/// Hook which is called upon reception of a class request with IN data stage
//
void usbcrHookProcessIn(void);

//
/// Hook which is called upon reception of a vendor request with OUT data stage
//
void usbvrHookProcessOut(void);

//
/// Hook which is called upon reception of a vendor request with IN data stage
//
void usbvrHookProcessIn(void);

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Endpoint Access Macros
/// Note that the endpoint control registers are indexed, meaning that an endpoint must be selected
/// before the control operations listed below can be used. Interrupts using any of these macros, must
/// save the current selection and restore it upon return.
//@{

//
/// Selects which IN/OUT endpoint (by index 0 to 5) to operate on
//
#define USBFW_SELECT_ENDPOINT(n)            (HWREG(USB_INDEX) = (n))

//
/// Gets the currently selected IN/OUT endpoint
//
#define USBFW_GET_SELECTED_ENDPOINT()       HWREG(USB_INDEX)

//
/// Stalls the selected IN endpoint
//
#define USBFW_STALL_IN_ENDPOINT() st (\
    USBCSIL = USBCSIL_SEND_STALL; \
    usbfwData.pEpInStatus[USBFW_GET_SELECTED_ENDPOINT() - 1] = EP_HALT; )

//
/// Returns the stall condition for the selected IN endpoint
//
#define USBFW_IN_ENDPOINT_STALLED()         (HWREG(USB_CSIL) & USB_CSIL_SENDSTALL_M)

//
/// Flushes the FIFO for the selected IN endpoint (flush twice when using double-buffering)
//
#define USBFW_FLUSH_IN_ENDPOINT()           (HWREG(USB_CSIL) = USB_CSIL_FLUSHPACKET)

//
/// Arms the selected IN endpoint, so that contents of the endpoint FIFO can be sent to the host
//
#define USBFW_ARM_IN_ENDPOINT()             (HWREG(USB_CSIL) = USB_CSIL_INPKTRDY)

//
/// Is the selected IN endpoint disarmed?
//
#define USBFW_IN_ENDPOINT_DISARMED()        !(HWREG(USB_CSIL) & USB_CSIL_INPKTRDY_M)

//
/// Is the FIFO for the selected IN endpoint empty?
//
#define USBFW_IN_ENDPOINT_FIFO_EMPTY()      !(HWREG(USB_CSIL) & USB_CSIL_PKTPRESENT_M)

//
/// Is the selected IN endpoint not currently being flushed?
//
#define USBFW_IN_ENDPOINT_NOT_FLUSHING()    !(HWREG(USB_CSIL) & USB_CSIL_FLUSHPACKET)

//
/// Stalls the selected OUT endpoint
//
#define USBFW_STALL_OUT_ENDPOINT() st ( \
    USBCSOL = USBCSOL_SEND_STALL; \
    usbfwData.pEpOutStatus[USBFW_GET_SELECTED_ENDPOINT() - 1] = EP_HALT; \
)

//
/// Returns the stall condition for the selected OUT endpoint
//
#define USBFW_OUT_ENDPOINT_STALLED()        (HWREG(USB_CSOL) & USB_CSOL_SENDSTALL_M)

//
/// Flushes the FIFO for the selected OUT endpoint (flush twice when using double-buffering)
//
#define USBFW_FLUSH_OUT_ENDPOINT()          (HWREG(USB_CSOL) = USB_CSOL_FLUSHPACKET)

//
/// Arms the selected OUT endpoint, so that the FIFO can receive data from the host
//
#define USBFW_ARM_OUT_ENDPOINT()            (HWREG(USB_CSOL) = 0x00)

//
/// Is the selected OUT endpoint disarmed? If so, there is data waiting in the FIFO
//
#define USBFW_OUT_ENDPOINT_DISARMED()       (HWREG(USB_CSOL) & USB_CSOL_OUTPKTRDY_M)

//
/// Is the selected OUT endpoint not currently being flushed?
//
#define USBFW_OUT_ENDPOINT_NOT_FLUSHING()   !(HWREG(USB_CSOL) & USB_CSOL_FLUSHPACKET_M)

//
/// Returns the number of bytes currently in the FIFO of the selected OUT endpoint, low byte
//
#define USBFW_GET_OUT_ENDPOINT_COUNT_LOW()  (HWREG(USB_CNTL))

//
/// Returns the number of bytes currently in the FIFO of the selected OUT endpoint, high byte
//
#define USBFW_GET_OUT_ENDPOINT_COUNT_HIGH() (HWREG(USB_CNTH))

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Function prototypes
void usbfwInit(void);
void usbfwResetHandler(void);
void usbfwSetupHandler(void);
void usbfwSetAllEpStatus(EP_STATUS status);
void usbfwWriteFifo(uint32_t fifoReg, uint32_t count, void *pData);
void usbfwReadFifo(uint32_t fifoReg, uint32_t count, void *pData);
//-------------------------------------------------------------------------------------------------------

//@}

#endif // __USB_FRAMEWORK_H__
