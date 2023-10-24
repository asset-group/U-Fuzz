//*****************************************************************************
//! @file       usb_in_buffer.h
//! @brief      USB IN FIFO Ring Buffer.
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

#ifndef __USB_IN_BUFFER_H__
#define __USB_IN_BUFFER_H__
/** \addtogroup module_usb_in_buffer USB IN FIFO Ring Buffer (usbibuf)
 * \ingroup module_usb_framework
 *
 * \brief Implements a ring buffer for IN endpoints
 *
 * The buffering mechanism is used to let the application operate on data streams containing data chunks
 * of arbitrary length, independent of how many USB packets they consist of, and abstracts away endpoint 
 * FIFO arming.
 * 
 * The application creates one instance of \ref USB_EPIN_RINGBUFFER_DATA for each USB IN endpoint to be
 * accessed this way. This data structure must be initialized by the application as indicated.
 *
 * All \ref module_usb_in_buffer functions must be called from the same preemption level, e.g. main 
 * context.
 *
 * \section usb_in_buffer_production Data Production
 * To push data to the ring-buffer for transfer to the USB host, the application calls:
 * - \ref usbibufGetMaxPushCount() to find the maximum number of bytes that can be written
 * - \ref usbibufPush() to write the desired number of bytes, given that this does not exceed the current
 *   maximum
 *
 * \section usb_in_buffer_consumption Data Consumption
 * The application polls usbibufPollEndpoint() to transfer data from the ring-buffer to the USB endpoint. 
 * Each call can result in 0 or more USB transactions, depending on endpoint double-buffering. 
 * Transaction sizes are maximum, independent of the data production.
 *
 * @{
 */

//
/// IN endpoint ring buffer instance
//
typedef __packed struct {
    uint16_t size;          ///< Number of bytes in total in the ring-buffer (must be initialized)
    uint16_t count;         ///< Number of bytes currently stored in the ring-buffer (initialize to 0)
    uint16_t head;          ///< Ring-buffer head byte index for software input (initialize to 0)
    uint16_t tail;          ///< Ring-buffer tail byte index for USB output (initialize to 0)
    uint8_t* pBuffer;       ///< Pointer to externally declared buffer memory (must be initialized)
    uint32_t endpointReg;   ///< Endpoint FIFO register address, e.g. USB_F2 (must be initialized)
    uint8_t  endpointIndex; ///< Endpoint index, n for USB_Fn (must be initialized)
    uint8_t  endpointSize;  ///< Maximum USB packet size for the endpoint (must be initialized)
} USB_EPIN_RINGBUFFER_DATA;     


//
// Function prototypes
//
void usbibufPollEndpoint(USB_EPIN_RINGBUFFER_DATA* pIbufData);
uint16_t usbibufGetMaxPushCount(USB_EPIN_RINGBUFFER_DATA* pIbufData);
void usbibufPush(USB_EPIN_RINGBUFFER_DATA* pIbufData, uint8_t* pSrc, uint16_t count);


//@}
#endif // __USB_IN_BUFFER_H__
