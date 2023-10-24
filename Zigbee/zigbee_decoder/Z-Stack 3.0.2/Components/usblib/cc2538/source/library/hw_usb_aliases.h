//*****************************************************************************
//! @file       hw_uab_aliases.h
//! @brief      Aliases for USB IP hardware registers.
//!
//! Revised     $Date: 2013-04-12 07:50:44 -0700 (Fri, 12 Apr 2013) $
//! Revision    $Revision: 9766 $
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

#ifndef __HW_USB_ALIASES_H__
#define __HW_USB_ALIASES_H__


//
// USB_CS0_CSIL aliases for endpoint 0
//
#define USB_CS0                     USB_CS0_CSIL

#define USB_CS0_CLRSETUPEND         0x00000080
#define USB_CS0_CLRSETUPEND_M       0x00000080
#define USB_CS0_CLRSETUPEND_S       7
#define USB_CS0_CLROUTPKTRDY        USB_CS0_CSIL_CLROUTPKTRDY_or_CLRDATATOG
#define USB_CS0_CLROUTPKTRDY_M      USB_CS0_CSIL_CLROUTPKTRDY_or_CLRDATATOG_M
#define USB_CS0_CLROUTPKTRDY_S      USB_CS0_CSIL_CLROUTPKTRDY_or_CLRDATATOG_S
#define USB_CS0_SENDSTALL           USB_CS0_CSIL_SENDSTALL_or_SENTSTALL
#define USB_CS0_SENDSTALL_M         USB_CS0_CSIL_SENDSTALL_or_SENTSTALL_M
#define USB_CS0_SENDSTALL_S         USB_CS0_CSIL_SENDSTALL_or_SENTSTALL_S
#define USB_CS0_SETUPEND            USB_CS0_CSIL_SETUPEND_or_SENDSTALL
#define USB_CS0_SETUPEND_M          USB_CS0_CSIL_SETUPEND_or_SENDSTALL_M
#define USB_CS0_SETUPEND_S          USB_CS0_CSIL_SETUPEND_or_SENDSTALL_S
#define USB_CS0_DATAEND             USB_CS0_CSIL_DATAEND_or_FLUSHPACKET
#define USB_CS0_DATAEND_M           USB_CS0_CSIL_DATAEND_or_FLUSHPACKET_M
#define USB_CS0_DATAEND_S           USB_CS0_CSIL_DATAEND_or_FLUSHPACKET_S
#define USB_CS0_SENTSTALL           USB_CS0_CSIL_SENTSTALL_or_UNDERRUN
#define USB_CS0_SENTSTALL_M         USB_CS0_CSIL_SENTSTALL_or_UNDERRUN_M
#define USB_CS0_SENTSTALL_S         USB_CS0_CSIL_SENTSTALL_or_UNDERRUN_S
#define USB_CS0_INPKTRDY            USB_CS0_CSIL_INPKTRDY_or_PKTPRESENT
#define USB_CS0_INPKTRDY_M          USB_CS0_CSIL_INPKTRDY_or_PKTPRESENT_M
#define USB_CS0_INPKTRDY_S          USB_CS0_CSIL_INPKTRDY_or_PKTPRESENT_S
#define USB_CS0_OUTPKTRDY           USB_CS0_CSIL_OUTPKTRDY_or_INPKTRDY
#define USB_CS0_OUTPKTRDY_M         USB_CS0_CSIL_OUTPKTRDY_or_INPKTRDY_M
#define USB_CS0_OUTPKTRDY_S         USB_CS0_CSIL_OUTPKTRDY_or_INPKTRDY_S


//
// USB_CS0_CSIL aliases for endpoints 1-5
//
#define USB_CSIL                    USB_CS0_CSIL

#define USB_CSIL_CLRDATATOG         USB_CS0_CSIL_CLROUTPKTRDY_or_CLRDATATOG
#define USB_CSIL_CLRDATATOG_M       USB_CS0_CSIL_CLROUTPKTRDY_or_CLRDATATOG_M
#define USB_CSIL_CLRDATATOG_S       USB_CS0_CSIL_CLROUTPKTRDY_or_CLRDATATOG_S
#define USB_CSIL_SENTSTALL          USB_CS0_CSIL_SENDSTALL_or_SENTSTALL
#define USB_CSIL_SENTSTALL_M        USB_CS0_CSIL_SENDSTALL_or_SENTSTALL_M
#define USB_CSIL_SENTSTALL_S        USB_CS0_CSIL_SENDSTALL_or_SENTSTALL_S
#define USB_CSIL_SENDSTALL          USB_CS0_CSIL_SETUPEND_or_SENDSTALL
#define USB_CSIL_SENDSTALL_M        USB_CS0_CSIL_SETUPEND_or_SENDSTALL_M
#define USB_CSIL_SENDSTALL_S        USB_CS0_CSIL_SETUPEND_or_SENDSTALL_S
#define USB_CSIL_FLUSHPACKET        USB_CS0_CSIL_DAT0END_or_FLUSHPACKET
#define USB_CSIL_FLUSHPACKET_M      USB_CS0_CSIL_DATAEND_or_FLUSHPACKET_M
#define USB_CSIL_FLUSHPACKET_S      USB_CS0_CSIL_DATAEND_or_FLUSHPACKET_S
#define USB_CSIL_UNDERRUN           USB_CS0_CSIL_SENTSTALL_or_UNDERRUN
#define USB_CSIL_UNDERRUN_M         USB_CS0_CSIL_SENTSTALL_or_UNDERRUN_M
#define USB_CSIL_UNDERRUN_S         USB_CS0_CSIL_SENTSTALL_or_UNDERRUN_S
#define USB_CSIL_PKTPRESENT         USB_CS0_CSIL_INPKTRDY_or_PKTPRESENT
#define USB_CSIL_PKTPRESENT_M       USB_CS0_CSIL_INPKTRDY_or_PKTPRESENT_M
#define USB_CSIL_PKTPRESENT_S       USB_CS0_CSIL_INPKTRDY_or_PKTPRESENT_S
#define USB_CSIL_INPKTRDY           USB_CS0_CSIL_OUTPKTRDY_or_INPKTRDY
#define USB_CSIL_INPKTRDY_M         USB_CS0_CSIL_OUTPKTRDY_or_INPKTRDY_M
#define USB_CSIL_INPKTRDY_S         USB_CS0_CSIL_OUTPKTRDY_or_INPKTRDY_S


//
// USB_CNT0_CNTL aliases for endpoint 0
//
#define USB_CNT0                    USB_CNT0_CNTL

#define USB_CNT0_FIFOCNT0_M         USB_CNT0_CNTL_USBCNT0_or_USBCNT_5_0_M
#define USB_CNT0_FIFOCNT0_S         USB_CNT0_CNTL_USBCNT0_or_USBCNT_5_0_S


//
// USB_CNT0_CNTL aliases for endpoints 1-5
//
#define USB_CNTL                    USB_CNT0_CNTL

#define USB_CNTL_FIFOCNT1TO5_M      USB_CNT0_CNTL_USBCNT0_or_USBCNT_5_0_M
#define USB_CNTL_FIFOCNT1TO5_S      USB_CNT0_CNTL_USBCNT0_or_USBCNT_5_0_S


#endif // __HW_USB_ALIASES_H__
