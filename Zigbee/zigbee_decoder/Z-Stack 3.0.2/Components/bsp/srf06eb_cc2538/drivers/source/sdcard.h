//*****************************************************************************
//! @file       sdcard.h
//! @brief      Header file for SD/MMC Cards. @note This header file
//!             does not include defines for all SD card registers.
//!
//! Revised     $Date: 2013-04-11 10:41:57 -0700 (Thu, 11 Apr 2013) $
//! Revision    $Revision: 9707 $
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
#ifndef __SDCARD_H__
#define __SDCARD_H__


/******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
 * INCLUDES
 */
#include "bsp.h"


/******************************************************************************
 * DEFINES
 */
 //
// Application specific defines
//
#define SDCARD_BLOCKLENGTH              512     //!< Number of bytes pr block
#define SDCARD_DUMMY                    0xFF
#define SDCARD_SPI_INIT_FREQ            400000  //!< Freq. during init (in Hz)
#define SDCARD_MAX_CMD_POLL_COUNT       0xFFF   //!< Max command poll count
#define SDCARD_MAX_RESPONSE_POLL_COUNT  0xFFFF  //!< Max response poll count

//
// Application specific defines
//
#define SDCARD_STATUS_M                 0x03    //!< Card status bitmask
#define SDCARD_STATUS_NOCARD            0       //!< No card present
#define SDCARD_STATUS_NOINIT            1       //!< Card present, not init'd
#define SDCARD_STATUS_READY             2       //!< Card present and ready
#define SDCARD_TYPE_MMCV3               0x10    //!< Card is MMCv3
#define SDCARD_TYPE_SDV1                0x20    //!< Card is SDv1
#define SDCARD_TYPE_SDV2                0x40    //!< Card is SDv2
#define SDCARD_BLOCK_ADDR               0x80    //!< Card uses LBA

#define SDCARD_SUCCESS                  0
#define SDCARD_ERROR                    1
#define SDCARD_ILLEGAL_CMD              2
#define SDCARD_DATA_READY_TIMEOUT       3
#define SDCARD_IDLE_STATE_TIMEOUT       4
#define SDCARD_SEND_OP_COND_TIMEOUT     5
#define SDCARD_SET_BLOCKLEN_TIMEOUT     6
#define SDCARD_WRITE_BLOCK_TIMEOUT      7
#define SDCARD_READ_BLOCK_TIMEOUT       8
#define SDCARD_SEND_CSD_TIMEOUT         9
#define SDCARD_SEND_CID_TIMEOUT         10
#define SDCARD_SD_STATUS_TIMEOUT        11

//
// Data transfer tokens
//
#define SDCARD_SINGLE_DATA_READY        0xFE    //!< Single transfer: Data ready
#define SDCARD_MULTI_BLOCKSTART         0xFC    //!< Start of block (multi send)
#define SDCARD_MULTI_STOP               0xFD    //!< End of multi send to card

//
// SD Card Command Respond Types
//
#define SDCARD_R1                       1       //!< 1 B response
#define SDCARD_R2                       2       //!< 2 B response
#define SDCARD_R3                       5       //!< 5 B response

//
// SD Card Basic Commands (selection)
//
#define SDCARD_CMD0                     0       //!< Response type R1
#define SDCARD_CMD1                     1       //!< Response type R1
#define SDCARD_CMD8                     8       //!< Response type R3
#define SDCARD_CMD9                     9       //!< Response type R1
#define SDCARD_CMD10                    10      //!< Response type R1
#define SDCARD_CMD12                    12
#define SDCARD_CMD13                    13      //!< Response type R2
#define SDCARD_CMD16                    16      //!< Response type R1
#define SDCARD_CMD17                    17      //!< Response type R1
#define SDCARD_CMD18                    18
#define SDCARD_CMD24                    24      //!< Response type R1
#define SDCARD_CMD25                    25
#define SDCARD_CMD27                    27
#define SDCARD_CMD28                    28
#define SDCARD_CMD29                    29
#define SDCARD_CMD30                    30
#define SDCARD_CMD32                    32
#define SDCARD_CMD33                    33
#define SDCARD_CMD38                    38
#define SDCARD_CMD55                    55      //!< Response type R1
#define SDCARD_CMD56                    56
#define SDCARD_CMD58                    58
#define SDCARD_CMD59                    59

//
// SD Card Basic Commands Abbreviations
//
#define SDCARD_GO_IDLE_STATE           SDCARD_CMD0
#define SDCARD_SEND_OP_COND            SDCARD_CMD1
#define SDCARD_SEND_CSD                SDCARD_CMD9
#define SDCARD_SEND_CID                SDCARD_CMD10
#define SDCARD_STOP_TRANSMISSION       SDCARD_CMD12
#define SDCARD_SEND_STATUS             SDCARD_CMD13
#define SDCARD_SET_BLOCKLEN            SDCARD_CMD16
#define SDCARD_READ_SINGLE_BLOCK       SDCARD_CMD17
#define SDCARD_READ_MULTIPLE_BLOCK     SDCARD_CMD18
#define SDCARD_WRITE_BLOCK             SDCARD_CMD24
#define SDCARD_WRITE_MULTIPLE_BLOCK    SDCARD_CMD25
#define SDCARD_PROGRAM_CSD             SDCARD_CMD27
#define SDCARD_SET_WRITE_PROT          SDCARD_CMD28
#define SDCARD_CLR_WRITE_PROT          SDCARD_CMD29
#define SDCARD_SEND_WRITE_PROT         SDCARD_CMD30
#define SDCARD_ERASE_WR_BLK_START      SDCARD_CMD32
#define SDCARD_ERASE_WR_BLK_END        SDCARD_CMD33
#define SDCARD_ERASE                   SDCARD_CMD38
#define SDCARD_APP_CMD                 SDCARD_CMD55
#define SDCARD_GEN_CMD                 SDCARD_CMD56
#define SDCARD_READ_OCR                SDCARD_CMD58
#define SDCARD_CRC_ON_OFF              SDCARD_CMD59

//
// SD Card Application Specific Commands
//
#define SDCARD_ACMD13                  13
#define SDCARD_ACMD18                  18
#define SDCARD_ACMD22                  22
#define SDCARD_ACMD23                  23
#define SDCARD_ACMD41                  41
#define SDCARD_ACMD42                  42
#define SDCARD_ACMD51                  51

//
// SD Card Application Specific Commands Abbreviations
//
#define SDCARD_SD_STATUS               SDCARD_ACMD13
#define SDCARD_SEND_NUM_WR_BLOCKS      SDCARD_ACMD22
#define SDCARD_SET_WR_BLK_ERASE_COUNT  SDCARD_ACMD23
#define SDCARD_SD_SEND_OP_COND         SDCARD_ACMD41
#define SDCARD_SET_CLR_CARD_DETECT     SDCARD_ACMD42
#define SDCARD_SEND_SCR                SDCARD_ACMD51


/******************************************************************************
* TYPES
*/
typedef struct
{
    //
    //! Manufacturer ID
    //
    uint8_t ui8Mid;

    //
    //! OEM/Application ID
    //
    uint8_t pui8Pid[2];

    //
    //! Product Name (ASCII characters)
    //
    char pcNm[5];

    //
    //! Product Revision
    //
    uint8_t ui8Rev;

    //
    //! Serial Number
    //
    uint32_t ui32Psn;

    //
    //! Manufacturing year. Offset from 2000
    //
    uint8_t ui8MdtY;

    //
    //! Manufacturing month. For example 04=April
    //
    uint8_t ui8MdtM;
}
tSdCardCid;


/******************************************************************************
* FUNCTION PROTOTYPES
*/
extern uint8_t sdCardInit(void);
extern uint8_t sdCardStatus(void);
extern uint8_t sdCardBlockRead(uint32_t ui32Block, uint8_t *pui8Buffer);
extern uint8_t sdCardBlockWrite(uint32_t ui32Block, const uint8_t *pcBuffer);
extern uint32_t sdCardGetBlockSize(void);
extern uint8_t sdCardGetStatusReg(uint8_t *pui8Buffer);
extern uint8_t sdCardGetCid(tSdCardCid *psCid);
extern uint8_t sdCardGetCsd(uint8_t *pui8Csd);
extern uint32_t sdCardGetSize(void);


/******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // __SDCARD_H__
