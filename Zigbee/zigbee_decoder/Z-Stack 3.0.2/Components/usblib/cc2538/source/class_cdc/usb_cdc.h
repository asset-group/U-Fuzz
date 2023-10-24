//*****************************************************************************
//! @file       usb_cdc.h
//! @brief      USB CDC definitions.
//!
//! Revised     $Date: 2013-02-25 05:00:56 -0800 (Mon, 25 Feb 2013) $
//! Revision    $Revision: 9379 $
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

#ifndef __USB_CDC_H__
#define __USB_CDC_H__


/* Data Interface Class Protocol Codes */
#define NO_PROTOCOL                 0x00    // No class specific protocol required


/* Communication Feature Selector Codes */
#define ABSTRACT_STATE              0x01
#define COUNTRY_SETTING             0x02

/* bDescSubType in Functional Descriptors */
#define USBCDC_FUNCDESC_HEADER                  0x00
#define USBCDC_FUNCDESC_CALL_MGMT               0x01
#define USBCDC_FUNCDESC_ABS_CTRL_MGMT           0x02
#define DSC_FN_DLM                              0x03    // DLM - Direct Line Managment
#define DSC_FN_TELEPHONE_RINGER                 0x04
#define DSC_FN_RPT_CAPABILITIES                 0x05
#define USBCDC_FUNCDESC_UNION_IF                0x06
#define DSC_FN_COUNTRY_SELECTION                0x07
#define DSC_FN_TEL_OP_MODES                     0x08
#define DSC_FN_USB_TERMINAL                     0x09
/* more.... see Table 25 in USB CDC Specification 1.1 */


#define CDC_COMM_INTF_ID            0x00
#define CDC_DATA_INTF_ID            0x01

// CLASS REQUESTS
#define USBCDC_REQ_SEND_ENCAPSULATED_COMMAND    0x00
#define USBCDC_REQ_GET_ENCAPSULATED_RESPONSE    0x01
#define USBCDC_REQ_SET_COMM_FEATURE             0x02     //optional
#define USBCDC_REQ_GET_COMM_FEATURE             0x03     //optional
#define USBCDC_REQ_CLEAR_COMM_FEATURE           0x04     //optional
#define USBCDC_REQ_SET_LINE_CODING              0x20     //optional
#define USBCDC_REQ_GET_LINE_CODING              0x21     //optional
#define USBCDC_REQ_SET_CONTROL_LINE_STATE       0x22     //optional
#define USBCDC_REQ_SEND_BREAK                   0x23     //optional



#define USBCDC_CHAR_FORMAT_1_STOP_BIT           0
#define USBCDC_CHAR_FORMAT_1_5_STOP_BIT         1
#define USBCDC_CHAR_FORMAT_2_STOP_BIT           2

#define USBCDC_PARITY_TYPE_NONE                 0
#define USBCDC_PARITY_TYPE_ODD                  1
#define USBCDC_PARITY_TYPE_EVEN                 2
#define USBCDC_PARITY_TYPE_MARK                 3
#define USBCDC_PARITY_TYPE_SPACE                4


#define USB_MAX_PACKET_SIZE             64  // As set in USB endpoint descriptor
#define UART_SET_RTS_LIMIT              200
#define UART_RELEASE_RTS_LIMIT          100

#define UART_FLOW_CTRL_STOP             1
#define UART_FLOW_CTRL_GO               0

#define UART_TX_BUFFER_EMPTY            0x01
#define UART_TX_STOPED_BY_FLOW_CTRL     0x02




typedef __packed struct {
    uint32_t dteRate;
    uint8_t  charFormat;
    uint8_t  parityType;
    uint8_t  dataBits;
} USBCDC_LINE_CODING;


typedef __packed struct {
    USBCDC_LINE_CODING lineCoding;
    uint8_t  rts;
    uint8_t  cts;
} USBCDC_DATA;

extern USBCDC_DATA usbCdcData;


void usbCdcInit(uint32_t baudrate);
void usbCdcProcessEvents(void);


#endif // __USB_CDC_H__
