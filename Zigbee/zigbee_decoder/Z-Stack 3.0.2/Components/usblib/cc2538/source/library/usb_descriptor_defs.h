//*****************************************************************************
//! @file       usb_descriptor_defs.h
//! @brief      USB descriptor definitions.
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

#ifndef __USB_DESCRIPTOR_DEFS_H__
#define __USB_DESCRIPTOR_DEFS_H__
/** \addtogroup module_usb_descriptor USB Descriptor Definitions
 *
 * \brief This module contains USB descriptor definitions and guidelines for writing USB descriptor sets
 *
 * The descriptor framework is fully generic and can support any USB device with static USB descriptors.
 *
 * The USB descriptor set is accessed internally by the \ref module_usb_standard_requests, using the
 * \ref module_usb_descriptor_parser. There is no need for the application code to access the descriptor
 * set, but a descriptor set must be defined for each application since it identifies the application
 * uniquely and is application- or class-specific.
 *
 * Detailed information on the specific USB descriptor types is available in the USB 2.0 specification
 * and in device class documentation. Examples of complete descriptor sets can be found e.g. in the HID
 * and CDC class examples.
 *
 *
 * \section section_usbdesc_guidelines Guidelines for Writing the USB Descriptor Set
 * The USB device firmware library interfaces with the application-specific USB descriptor set through
 * one user-written C header file, \b app_usb_descriptor.h and one user-written C source file,
 * \b app_usb_descriptor.c. These files contain:
 * - String descriptor type definitions, as they depend on the string lengths:
 *     \code
 *     // Defines the USB string descriptor struct types (index, number of unicode characters)
 *     TYPEDEF_USB_STRING_DESCRIPTOR(0, 1);  // Language IDs
 *     TYPEDEF_USB_STRING_DESCRIPTOR(1, 16); // Manufacturer
 *     ...
 *     \endcode
 * - Type definition and declaration of standard formatted USB descriptors (starting with length and type
 *   bytes, e.g. \ref USB_DEVICE_DESCRIPTOR)
 *     \code
 *     typedef __packed struct {
 *         USB_DEVICE_DESCRIPTOR device;
 *         ...
 *     } USB_DESCRIPTOR;
 *
 *     const USB_DESCRIPTOR usbDescriptor = {
 *         ...
 *     };
 *     \endcode
 * - Declaration of non-standard formatted USB descriptors as byte arrays, e.g. HID report descriptors
 *     \code
 *     /// Keyboard report descriptor (using format for Boot interface descriptor)
 *     static const uint8_t pHid0ReportDesc[] = {
 *         0x05, 0x01,    // Usage Pg (Generic Desktop)
 *         0x09, 0x06,    // Usage (Keyboard)
 *         0xA1, 0x01,    // Collection: (Application)
 *         ...
 *         0xC0           // End Collection
 *     };
 *     \endcode
 * - A look-up table on the GET_DESCRIPTOR request's index and value parameters for USB descriptors other
 *   than \ref USB_DEVICE_DESCRIPTOR and \ref USB_CONFIGURATION_DESCRIPTOR. This includes:
 *     - Non-standard formatted descriptors (\b not starting with length and type bytes, e.g. HID resport
 *       descriptors)
 *     - Standard formatted string descriptors, to be able to support language IDs
 *     - Standard formatted class-specific descriptors within the configuration descriptor, e.g.
 *       \ref USBHID_DESCRIPTOR
 *     \code
 *     const USB_DESCRIPTOR_LUT pUsbDescriptorLut[] = {
 *     //    value   index   length                           pDesc
 *         { 0x0300, 0x0000, sizeof(USB_STRING_0_DESCRIPTOR), &usbDescriptor.strings.langIds },
 *         { 0x0301, 0x0409, sizeof(USB_STRING_1_DESCRIPTOR), &usbDescriptor.strings.manufacturer },
 *         ...
 *         { 0x2200, 0x0000, sizeof(pHid0ReportDesc),         &pHid0ReportDesc },
 *         ...
 *         { 0x0000, 0x0000, 0,                               NULL }
 *     };
 *     \endcode
 * - A look-up table specifying which endpoints use double-buffering, with one entry for each
 *   \ref USB_INTERFACE_DESCRIPTOR in the descriptor set. For each interface descriptor, there is one
 *   bit-mask for IN endpoints and one for OUT endpoints, in which bit N maps to endpoint N.
 *     \code
 *     const USB_INTERFACE_EP_DBLBUF_LUT pUsbInterfaceEpDblbufLut[] = {
 *     //    pInterface                 inMask  outMask
 *         { &usbDescriptor.interface0, 0x0002, 0x0008 }, // EP1 IN and EP3 OUT are double-buffered
 *         { &usbDescriptor.interface1, 0x0000, 0x0000 }  // No endpoints are double-buffered
 *     };
 *     \endcode
 *
 *
 * \subsection section_usbdesc_descriptors USB Descriptors
 * The main USB descriptor set containing standard formatted descriptors must be defined and declared as
 * follows:
 * - Device descriptor (\ref USB_DEVICE_DESCRIPTOR)
 *     - Configuration descriptor (\ref USB_CONFIGURATION_DESCRIPTOR)
 *         - Interface descriptor (\ref USB_INTERFACE_DESCRIPTOR)
 *             - Class specific descriptors, if any
 *             - Endpoint descriptors, if any (\ref USB_ENDPOINT_DESCRIPTOR)
 *             - More class specific descriptors, if any
 *         - More interface descriptors, if any
 *     - More configuration descriptors, if any
 * - String descriptors, if any (\ref USB_STRING_DESCRIPTOR)
 *
 * The descriptor set struct definition shall be named \b USB_DESCRIPTOR. The descriptor set shall be
 * <b>const USB_DESCRIPTOR usbDescriptor</b>, and must be declared in flash memory or ROM. Dynamic
 * string descriptors, such as serial number, can be declared separately in RAM and be initialized before
 * the USB interface is activate.
 *
 * @{
 */

#include <stdint.h>


//-------------------------------------------------------------------------------------------------------
/// \name Sizes
//@{
#define USB_EP0_PACKET_SIZE             32    ///< The maximum data packet size for endpoint 0
//@}

/// \name Standard Descriptor Types
//@{
#define USB_DESC_TYPE_DEVICE            0x01  ///< Device
#define USB_DESC_TYPE_CONFIG            0x02  ///< Configuration
#define USB_DESC_TYPE_STRING            0x03  ///< String
#define USB_DESC_TYPE_INTERFACE         0x04  ///< Interface
#define USB_DESC_TYPE_ENDPOINT          0x05  ///< Endpoint
//@}

/// \name Generic Class Descriptor Types
//@{
#define USB_DESC_TYPE_CS_INTERFACE      0x24  ///< Class specific interface descriptor
#define USB_DESC_TYPE_CS_ENDPOINT       0x25  ///< Class specific endpoint descriptor
//@}

/// \name HID Class Descriptor Types
//@{
#define USB_DESC_TYPE_HID               0x21  ///< HID descriptor
#define USB_DESC_TYPE_HIDREPORT         0x22  ///< Report descriptor
//@}

/// \name CDC Class Descriptor Subtypes
//@{
#define USBCDC_FUNCDESC_HEADER          0x00  ///< Header function descriptor
#define USBCDC_FUNCDESC_CALL_MGMT       0x01  ///< Call management function descriptor
#define USBCDC_FUNCDESC_ABS_CTRL_MGMT   0x02  ///< Abstract control management function descriptor
#define USBCDC_FUNCDESC_UNION_IF        0x06  ///< Union interface function descriptor
//@}

/// \name Endpoint Types
//@{
#define USB_EP_ATTR_CTRL                0x00  ///< Control (endpoint 0 only)
#define USB_EP_ATTR_ISO                 0x01  ///< Isochronous (not acknowledged)
#define USB_EP_ATTR_BULK                0x02  ///< Bulk
#define USB_EP_ATTR_INT                 0x03  ///< Interrupt (guaranteed polling interval)
#define USB_EP_ATTR_TYPE_BM             0x03  ///< Endpoint type bitmask
//@}

//-------------------------------------------------------------------------------------------------------


/// USB device descriptor
typedef __packed struct {
    uint8_t  bLength;             ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_DEVICE
    uint16_t bcdUSB;              ///< USB specification release number (in BCD, e.g. 0110 for USB 1.1)
    uint8_t  bDeviceClass;        ///< Device class code
    uint8_t  bDeviceSubClass;     ///< Device subclass code
    uint8_t  bDeviceProtocol;     ///< Device protocol code
    uint8_t  bMaxPacketSize0;     ///< Maximum packet size for EP0
    uint16_t idVendor;            ///< Vendor ID
    uint16_t idProduct;           ///< Product ID
    uint16_t bcdDevice;           ///< Device release number (in BCD)
    uint8_t  iManufacturer;       ///< Index of the string descriptor for manufacturer
    uint8_t  iProduct;            ///< Index of the string descriptor for product
    uint8_t  iSerialNumber;       ///< Index of the string descriptor for serial number
    uint8_t  bNumConfigurations;  ///< Number of possible configurations
} USB_DEVICE_DESCRIPTOR;

/// USB configuration descriptor
typedef __packed struct {
    uint8_t  bLength;             ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_CONFIG
    uint16_t wTotalLength;        ///< Total length of data for this configuration
    uint8_t  bNumInterfaces;      ///< Number of interfaces supported by this configuration (one-based index)
    uint8_t  bConfigurationValue; ///< Designator value for this configuration
    uint8_t  iConfiguration;      ///< Index of the string descriptor for this configuration
    uint8_t  bmAttributes;        ///< Configuration characteristics
    uint8_t  bMaxPower;           ///< Maximum power consumption in this configuration (bMaxPower * 2 mA)
} USB_CONFIGURATION_DESCRIPTOR;

/// USB interface descriptor
typedef __packed struct {
    uint8_t  bLength;             ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_INTERFACE
    uint8_t  bInterfaceNumber;    ///< Number of *this* interface (zero-based index)
    uint8_t  bAlternateSetting;   ///< Alternate setting index for this descriptor (zero-based index)
    uint8_t  bNumEndpoints;       ///< Number of endpoints for this interface (excl. EP0)
    uint8_t  bInterfaceClass;     ///< Interface class code
    uint8_t  bInterfaceSubClass;  ///< Interface subclass code
    uint8_t  bInterfaceProtocol;  ///< Interface protocol code
    uint8_t  iInterface;          ///< Index of the string descriptor for this interface
} USB_INTERFACE_DESCRIPTOR;

/// USB endpoint descriptor
typedef __packed struct {
    uint8_t  bLength;             ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_ENDPOINT
    uint8_t  bEndpointAddress;    ///< Endpoint address (direction[7] + number[3:0])
    uint8_t  bmAttributes;        ///< Endpoint attributes (ISO / BULK / INT)
    uint16_t wMaxPacketSize;      ///< Maximum endpoint packet size
    uint8_t  bInterval;           ///< \ref USB_EP_ATTR_INT : Polling interval (in ms)
} USB_ENDPOINT_DESCRIPTOR;

/// USB string descriptor
typedef __packed struct { \
    uint8_t  bLength;             ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_STRING
    uint16_t pString[1];          ///< Unicode string
} USB_STRING_DESCRIPTOR;

/// Creates type definition for USB string descriptor with arbitrary unicode character count
#define TYPEDEF_USB_STRING_DESCRIPTOR(index, charCount) \
    typedef __packed struct { \
        uint8_t  bLength;             /* Size of this descriptor (in bytes) */ \
        uint8_t  bDescriptorType;     /* Descriptor type = \ref USB_DESC_TYPE_STRING */ \
        uint16_t pString[charCount];  /* Unicode string */ \
    } USB_STRING_##index##_DESCRIPTOR




/// USB HID descriptor
typedef __packed struct {
    uint8_t  bLength;             ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_HID
    uint16_t bscHID;              ///< HID specification release number (in BCD)
    uint8_t  bCountryCode;        ///< Hardware target country
    uint8_t  bNumDescriptors;     ///< Number of HID class descriptors to follow
    uint8_t  bRDescriptorType;    ///< Report descriptor type
    uint16_t wDescriptorLength;   ///< Total length of the associated report descriptor
} USBHID_DESCRIPTOR;




/// USB CDC header functional descriptor
typedef __packed struct { \
    uint8_t  bFunctionLength;     ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_CS_INTERFACE
    uint8_t  bDescriptorSubType;  ///< Descriptor subtype = \ref USBCDC_FUNCDESC_HEADER
    uint16_t bcdCDC;              ///< CDC specification release number (in BCD)
} USBCDC_HEADER_FUNC_DESCRIPTOR;

/// USB CDC abstract control management functional descriptor
typedef __packed struct { \
    uint8_t  bFunctionLength;     ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_CS_INTERFACE
    uint8_t  bDescriptorSubType;  ///< Descriptor subtype = \ref USBCDC_FUNCDESC_ABS_CTRL_MGMT
    uint8_t  bmCapabilities;      ///< The capabilities that this configuration supports
} USBCDC_ABSTRACT_CTRL_MGMT_FUNC_DESCRIPTOR;

/// USB CDC union interface functional descriptor
typedef __packed struct { \
    uint8_t  bFunctionLength;     ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_CS_INTERFACE
    uint8_t  bDescriptorSubType;  ///< Descriptor subtype = \ref USBCDC_FUNCDESC_UNION_IF
    uint8_t  bMasterInterface;    ///< The interface number of the Communication or Data Class interface, designated as the master or controlling interface for the union
    uint8_t  bSlaveInterface0;    ///< Interface number of first slave or associated interface in the union
} USBCDC_UNION_INTERFACE_FUNC_DESCRIPTOR;

/// USB CDC call management functional descriptor
typedef __packed struct { \
    uint8_t  bFunctionLength;     ///< Size of this descriptor (in bytes)
    uint8_t  bDescriptorType;     ///< Descriptor type = \ref USB_DESC_TYPE_CS_INTERFACE
    uint8_t  bDescriptorSubType;  ///< Descriptor subtype = \ref USBCDC_FUNCDESC_CALL_MGMT
    uint8_t  bmCapabilities;      ///< The capabilities that this configuration supports
    uint8_t  bDataInterface;      ///< Interface number of Data Class interface optionally used for call management
} USBCDC_CALL_MGMT_FUNC_DESCRIPTOR;




// Look-up table entry for descriptors other than device and configuration (table is NULL-terminated)
typedef __packed struct {
    uint16_t value;               ///< Value field to be matched
    uint16_t index;               ///< Index field to be matched
    uint16_t length;              ///< Length of the descriptor (maximum returned)
    const void* pDesc;            ///< Pointer to the descriptor data in flash or RAM
} USB_DESCRIPTOR_LUT;

// Look-up table entry for descriptors other than device and configuration (table is NULL-terminated)
extern const USB_DESCRIPTOR_LUT pUsbDescriptorLut[];


// Look-up table entry specifying double-buffering for each interface descriptor
typedef __packed struct {
    const USB_INTERFACE_DESCRIPTOR* pInterface; ///< Pointer to the interface descriptor
    uint16_t inMask;              ///< Bitmask for IN endpoints (bit x maps to EPx IN)
    uint16_t outMask;             ///< Bitmask for OUT endpoints (bit x maps to EPx OUT)
} USB_INTERFACE_EP_DBLBUF_LUT;

// Look-up table entry specifying double-buffering for each interface descriptor
extern const USB_INTERFACE_EP_DBLBUF_LUT pUsbInterfaceEpDblbufLut[];

//@}


#endif // __USB_DESCRIPTOR_DEFS_H__
