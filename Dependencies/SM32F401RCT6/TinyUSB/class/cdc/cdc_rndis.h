/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


/** \ingroup ClassDriver_CDC Communication Device Class (CDC)
 * \defgroup CDC_RNDIS Remote Network Driver Interface Specification (RNDIS)
 *  @{
 *  \defgroup CDC_RNDIS_Common Common Definitions
 *  @{ */

#ifndef _TUSB_CDC_RNDIS_H_
#define _TUSB_CDC_RNDIS_H_

#include "cdc.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __CC_ARM
#pragma diag_suppress 66 // Suppress Keil warnings #66-D: enumeration value is out of "int" range
#endif

/// RNDIS Message Types
typedef enum
{
  RNDIS_MSG_PACKET           = 0x00000001UL, ///< The host and device use this to send network data to one another.

  RNDIS_MSG_INITIALIZE       = 0x00000002UL, ///< Sent by the host to initialize the device.
  RNDIS_MSG_INITIALIZE_CMPLT = 0x80000002UL, ///< Device response to an initialize message.

  RNDIS_MSG_HALT             = 0x00000003UL, ///< Sent by the host to halt the device. This does not have a response. It is optional for the device to send this message to the host.

  RNDIS_MSG_QUERY            = 0x00000004UL, ///< Sent by the host to send a query OID.
  RNDIS_MSG_QUERY_CMPLT      = 0x80000004UL, ///< Device response to a query OID.

  RNDIS_MSG_SET              = 0x00000005UL, ///< Sent by the host to send a set OID.
  RNDIS_MSG_SET_CMPLT        = 0x80000005UL, ///< Device response to a set OID.

  RNDIS_MSG_RESET            = 0x00000006UL, ///< Sent by the host to perform a soft reset on the device.
  RNDIS_MSG_RESET_CMPLT      = 0x80000006UL, ///< Device response to reset message.

  RNDIS_MSG_INDICATE_STATUS  = 0x00000007UL, ///< Sent by the device to indicate its status or an error when an unrecognized message is received.

  RNDIS_MSG_KEEP_ALIVE       = 0x00000008UL, ///< During idle periods, sent every few seconds by the host to check that the device is still responsive. It is optional for the device to send this message to check if the host is active.
  RNDIS_MSG_KEEP_ALIVE_CMPLT = 0x80000008UL  ///< The device response to a keepalivemessage. The host can respond with this message to a keepalive message from the device when the device implements the optional KeepAliveTimer.
}rndis_msg_type_t;

/// RNDIS Message Status Values
typedef enum
{
  RNDIS_STATUS_SUCCESS          = 0x00000000UL, ///< Success
  RNDIS_STATUS_FAILURE          = 0xC0000001UL, ///< Unspecified error
  RNDIS_STATUS_INVALID_DATA     = 0xC0010015UL, ///< Invalid data error
  RNDIS_STATUS_NOT_SUPPORTED    = 0xC00000BBUL, ///< Unsupported request error
  RNDIS_STATUS_MEDIA_CONNECT    = 0x4001000BUL, ///< Device is connected to a network medium.
  RNDIS_STATUS_MEDIA_DISCONNECT = 0x4001000CUL  ///< Device is disconnected from the medium.
}rndis_msg_status_t;

#ifdef __CC_ARM
#pragma diag_default 66 // return Keil 66 to normal severity
#endif

//--------------------------------------------------------------------+
// MESSAGE STRUCTURE
//--------------------------------------------------------------------+

//------------- Initialize -------------//
/// \brief Initialize Message
/// \details This message MUST be sent by the host to initialize the device.
typedef struct {
  uint32_t type          ; ///< Message type, must be \ref RNDIS_MSG_INITIALIZE
  uint32_t length        ; ///< Message length in bytes, must be 0x18
  uint32_t request_id    ; ///< A 32-bit integer value, generated by the host, used to match the host's sent request to the response from the device.
  uint32_t major_version ; ///< The major version of the RNDIS Protocol implemented by the host.
  uint32_t minor_version ; ///< The minor version of the RNDIS Protocol implemented by the host
  uint32_t max_xfer_size ; ///< The maximum size, in bytes, of any single bus data transfer that the host expects to receive from the device.
}rndis_msg_initialize_t;

/// \brief Initialize Complete Message
/// \details This message MUST be sent by the device in response to an initialize message.
typedef struct {
  uint32_t type                    ; ///< Message Type, must be \ref RNDIS_MSG_INITIALIZE_CMPLT
  uint32_t length                  ; ///< Message length in bytes, must be 0x30
  uint32_t request_id              ; ///< A 32-bit integer value from \a request_id field of the \ref rndis_msg_initialize_t to which this message is a response.
  uint32_t status                  ; ///< The initialization status of the device, has value from \ref rndis_msg_status_t
  uint32_t major_version           ; ///< the highest-numbered RNDIS Protocol version supported by the device.
  uint32_t minor_version           ; ///< the highest-numbered RNDIS Protocol version supported by the device.
  uint32_t device_flags            ; ///< MUST be set to 0x000000010. Other values are reserved for future use.
  uint32_t medium                  ; ///< is 0x00 for RNDIS_MEDIUM_802_3
  uint32_t max_packet_per_xfer     ; ///< The maximum number of concatenated \ref RNDIS_MSG_PACKET messages that the device can handle in a single bus transfer to it. This value MUST be at least 1.
  uint32_t max_xfer_size           ; ///< The maximum size, in bytes, of any single bus data transfer that the device expects to receive from the host.
  uint32_t packet_alignment_factor ; ///< The byte alignment the device expects for each RNDIS message that is part of a multimessage transfer to it. The value is specified as an exponent of 2; for example, the host uses 2<SUP>{PacketAlignmentFactor}</SUP> as the alignment value.
  uint32_t reserved[2]             ;
} rndis_msg_initialize_cmplt_t;

//------------- Query -------------//
/// \brief Query Message
/// \details This message MUST be sent by the host to query an OID.
typedef struct {
  uint32_t type          ; ///< Message Type, must be \ref RNDIS_MSG_QUERY
  uint32_t length        ; ///< Message length in bytes, including the header and the \a oid_buffer
  uint32_t request_id    ; ///< A 32-bit integer value, generated by the host, used to match the host's sent request to the response from the device.
  uint32_t oid           ; ///< The integer value of the host operating system-defined identifier, for the parameter of the device being queried for.
  uint32_t buffer_length ; ///< The length, in bytes, of the input data required for the OID query. This MUST be set to 0 when there is no input data associated with the OID.
  uint32_t buffer_offset ; ///< The offset, in bytes, from the beginning of \a request_id field where the input data for the query is located in the message. This value MUST be set to 0 when there is no input data associated with the OID.
  uint32_t reserved      ;
  uint8_t  oid_buffer[]  ; ///< Flexible array contains the input data supplied by the host, required for the OID query request processing by the device, as per the host NDIS specification.
} rndis_msg_query_t, rndis_msg_set_t;

TU_VERIFY_STATIC(sizeof(rndis_msg_query_t) == 28, "Make sure flexible array member does not affect layout");

/// \brief Query Complete Message
/// \details This message MUST be sent by the device in response to a query OID message.
typedef struct {
  uint32_t type          ; ///< Message Type, must be \ref RNDIS_MSG_QUERY_CMPLT
  uint32_t length        ; ///< Message length in bytes, including the header and the \a oid_buffer
  uint32_t request_id    ; ///< A 32-bit integer value from \a request_id field of the \ref rndis_msg_query_t to which this message is a response.
  uint32_t status        ; ///< The status of processing for the query request, has value from \ref rndis_msg_status_t.
  uint32_t buffer_length ; ///< The length, in bytes, of the data in the response to the query. This MUST be set to 0 when there is no OIDInputBuffer.
  uint32_t buffer_offset ; ///< The offset, in bytes, from the beginning of \a request_id field where the response data for the query is located in the message. This MUST be set to 0 when there is no \ref oid_buffer.
  uint8_t  oid_buffer[]  ; ///< Flexible array member contains the response data to the OID query request as specified by the host.
} rndis_msg_query_cmplt_t;

TU_VERIFY_STATIC(sizeof(rndis_msg_query_cmplt_t) == 24, "Make sure flexible array member does not affect layout");

//------------- Reset -------------//
/// \brief Reset Message
/// \details This message MUST be sent by the host to perform a soft reset on the device.
typedef struct {
  uint32_t type     ; ///< Message Type, must be \ref RNDIS_MSG_RESET
  uint32_t length   ; ///< Message length in bytes, MUST be 0x06
  uint32_t reserved ;
} rndis_msg_reset_t;

/// \brief Reset Complete Message
/// \details This message MUST be sent by the device in response to a reset message.
typedef struct {
  uint32_t type             ; ///< Message Type, must be \ref RNDIS_MSG_RESET_CMPLT
  uint32_t length           ; ///< Message length in bytes, MUST be 0x10
  uint32_t status           ; ///< The status of processing for the \ref rndis_msg_reset_t, has value from \ref rndis_msg_status_t.
  uint32_t addressing_reset ; ///< This field indicates whether the addressing information, which is the multicast address list or packet filter, has been lost during the reset operation. This MUST be set to 0x00000001 if the device requires that the host to resend addressing information or MUST be set to zero otherwise.
} rndis_msg_reset_cmplt_t;

//typedef struct {
//  uint32_t type;
//  uint32_t length;
//  uint32_t status;
//  uint32_t buffer_length;
//  uint32_t buffer_offset;
//  uint32_t diagnostic_status; // optional
//  uint32_t diagnostic_error_offset; // optional
//  uint32_t status_buffer[0]; // optional
//} rndis_msg_indicate_status_t;

/// \brief Keep Alive Message
/// \details This message MUST be sent by the host to check that device is still responsive. It is optional for the device to send this message to check if the host is active
typedef struct {
  uint32_t type       ; ///< Message Type
  uint32_t length     ; ///< Message length in bytes, MUST be 0x10
  uint32_t request_id ;
} rndis_msg_keep_alive_t, rndis_msg_halt_t;

/// \brief Set Complete Message
/// \brief This message MUST be sent in response to a the request message
typedef struct {
  uint32_t type       ; ///< Message Type
  uint32_t length     ; ///< Message length in bytes, MUST be 0x10
  uint32_t request_id ; ///< must be the same as requesting message
  uint32_t status     ; ///< The status of processing for the request message request by the device to which this message is the response.
} rndis_msg_set_cmplt_t, rndis_msg_keep_alive_cmplt_t;

/// \brief Packet Data Message
/// \brief This message MUST be used by the host and the device to send network data to one another.
typedef struct {
  uint32_t type                          ; ///< Message Type, must be \ref RNDIS_MSG_PACKET
  uint32_t length                        ; ///< Message length in bytes, The total length of this RNDIS message including the header, payload, and padding.
  uint32_t data_offset                   ; ///< Specifies the offset, in bytes, from the start of this \a data_offset field of this message to the start of the data. This MUST be an integer multiple of 4.
  uint32_t data_length                   ; ///< Specifies the number of bytes in the payload of this message.
  uint32_t out_of_band_data_offet        ; ///< Specifies the offset, in bytes, of the first out-of-band data record from the start of the DataOffset field in this message. MUST be an integer multiple of 4 when out-of-band data is present or set to 0 otherwise. When there are multiple out-ofband data records, each subsequent record MUST immediately follow the previous out-of-band data record.
  uint32_t out_of_band_data_length       ; ///< Specifies, in bytes, the total length of the out-of-band data.
  uint32_t num_out_of_band_data_elements ; ///< Specifies the number of out-of-band records in this message.
  uint32_t per_packet_info_offset        ; ///< Specifies the offset, in bytes, of the start of per-packet-info data record from the start of the \a data_offset field in this message. MUST be an integer multiple of 4 when per-packet-info data record is present or MUST be set to 0 otherwise. When there are multiple per-packet-info data records, each subsequent record MUST immediately follow the previous record.
  uint32_t per_packet_info_length        ; ///< Specifies, in bytes, the total length of per-packetinformation contained in this message.
  uint32_t reserved[2]                   ;
  uint32_t payload[0]                    ; ///< Network data contained in this message.

  // uint8_t  padding[0]
  // Additional bytes of zeros added at the end of the message to comply with
  // the internal and external padding requirements. Internal padding SHOULD be as per the
  // specification of the out-of-band data record and per-packet-info data record. The external
  //padding size SHOULD be determined based on the PacketAlignmentFactor field specification
  //in REMOTE_NDIS_INITIALIZE_CMPLT message by the device, when multiple
  //REMOTE_NDIS_PACKET_MSG messages are bundled together in a single bus-native message.
  //In this case, all but the very last REMOTE_NDIS_PACKET_MSG MUST respect the
  //PacketAlignmentFactor field.

  // rndis_msg_packet_t [0] : (optional) more packet if multiple packet per bus transaction is supported
} rndis_msg_packet_t;


typedef struct {
  uint32_t size    ; ///< Length, in bytes, of this header and appended data and padding. This value MUST be an integer multiple of 4.
  uint32_t type    ; ///< MUST be as per host operating system specification.
  uint32_t offset  ; ///< The byte offset from the beginning of this record to the beginning of data.
  uint32_t data[0] ; ///< Flexible array contains data
} rndis_msg_out_of_band_data_t, rndis_msg_per_packet_info_t;

//--------------------------------------------------------------------+
// NDIS Object ID
//--------------------------------------------------------------------+

/// NDIS Object ID
typedef enum
{
  //------------- General Required OIDs -------------//
  RNDIS_OID_GEN_SUPPORTED_LIST          = 0x00010101, ///< List of supported OIDs
  RNDIS_OID_GEN_HARDWARE_STATUS         = 0x00010102, ///< Hardware status
  RNDIS_OID_GEN_MEDIA_SUPPORTED         = 0x00010103, ///< Media types supported (encoded)
  RNDIS_OID_GEN_MEDIA_IN_USE            = 0x00010104, ///< Media types in use (encoded)
  RNDIS_OID_GEN_MAXIMUM_LOOKAHEAD       = 0x00010105, ///<
  RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE      = 0x00010106, ///< Maximum frame size in bytes
  RNDIS_OID_GEN_LINK_SPEED              = 0x00010107, ///< Link speed in units of 100 bps
  RNDIS_OID_GEN_TRANSMIT_BUFFER_SPACE   = 0x00010108, ///< Transmit buffer space
  RNDIS_OID_GEN_RECEIVE_BUFFER_SPACE    = 0x00010109, ///< Receive buffer space
  RNDIS_OID_GEN_TRANSMIT_BLOCK_SIZE     = 0x0001010A, ///< Minimum amount of storage, in bytes, that a single packet occupies in the transmit buffer space of the NIC
  RNDIS_OID_GEN_RECEIVE_BLOCK_SIZE      = 0x0001010B, ///< Amount of storage, in bytes, that a single packet occupies in the receive buffer space of the NIC
  RNDIS_OID_GEN_VENDOR_ID               = 0x0001010C, ///< Vendor NIC code
  RNDIS_OID_GEN_VENDOR_DESCRIPTION      = 0x0001010D, ///< Vendor network card description
  RNDIS_OID_GEN_CURRENT_PACKET_FILTER   = 0x0001010E, ///< Current packet filter (encoded)
  RNDIS_OID_GEN_CURRENT_LOOKAHEAD       = 0x0001010F, ///< Current lookahead size in bytes
  RNDIS_OID_GEN_DRIVER_VERSION          = 0x00010110, ///< NDIS version number used by the driver
  RNDIS_OID_GEN_MAXIMUM_TOTAL_SIZE      = 0x00010111, ///< Maximum total packet length in bytes
  RNDIS_OID_GEN_PROTOCOL_OPTIONS        = 0x00010112, ///< Optional protocol flags (encoded)
  RNDIS_OID_GEN_MAC_OPTIONS             = 0x00010113, ///< Optional NIC flags (encoded)
  RNDIS_OID_GEN_MEDIA_CONNECT_STATUS    = 0x00010114, ///< Whether the NIC is connected to the network
  RNDIS_OID_GEN_MAXIMUM_SEND_PACKETS    = 0x00010115, ///< The maximum number of send packets the driver can accept per call to its MiniportSendPacketsfunction

  //------------- General Optional OIDs -------------//
  RNDIS_OID_GEN_VENDOR_DRIVER_VERSION   = 0x00010116, ///< Vendor-assigned version number of the driver
  RNDIS_OID_GEN_SUPPORTED_GUIDS         = 0x00010117, ///< The custom GUIDs (Globally Unique Identifier) supported by the miniport driver
  RNDIS_OID_GEN_NETWORK_LAYER_ADDRESSES = 0x00010118, ///< List of network-layer addresses associated with the binding between a transport and the driver
  RNDIS_OID_GEN_TRANSPORT_HEADER_OFFSET = 0x00010119, ///< Size of packets' additional headers
  RNDIS_OID_GEN_MEDIA_CAPABILITIES      = 0x00010201, ///<
  RNDIS_OID_GEN_PHYSICAL_MEDIUM         = 0x00010202, ///< Physical media supported by the miniport driver (encoded)

  //------------- 802.3 Objects (Ethernet) -------------//
  RNDIS_OID_802_3_PERMANENT_ADDRESS     = 0x01010101, ///< Permanent station address
  RNDIS_OID_802_3_CURRENT_ADDRESS       = 0x01010102, ///< Current station address
  RNDIS_OID_802_3_MULTICAST_LIST        = 0x01010103, ///< Current multicast address list
  RNDIS_OID_802_3_MAXIMUM_LIST_SIZE     = 0x01010104, ///< Maximum size of multicast address list
} rndis_oid_type_t;

/// RNDIS Packet Filter Bits \ref RNDIS_OID_GEN_CURRENT_PACKET_FILTER.
typedef enum
{
  RNDIS_PACKET_TYPE_DIRECTED              = 0x00000001, ///< Directed packets. Directed packets contain a destination address equal to the station address of the NIC.
  RNDIS_PACKET_TYPE_MULTICAST             = 0x00000002, ///< Multicast address packets sent to addresses in the multicast address list.
  RNDIS_PACKET_TYPE_ALL_MULTICAST         = 0x00000004, ///< All multicast address packets, not just the ones enumerated in the multicast address list.
  RNDIS_PACKET_TYPE_BROADCAST             = 0x00000008, ///< Broadcast packets.
  RNDIS_PACKET_TYPE_SOURCE_ROUTING        = 0x00000010, ///< All source routing packets. If the protocol driver sets this bit, the NDIS library attempts to act as a source routing bridge.
  RNDIS_PACKET_TYPE_PROMISCUOUS           = 0x00000020, ///< Specifies all packets regardless of whether VLAN filtering is enabled or not and whether the VLAN identifier matches or not.
  RNDIS_PACKET_TYPE_SMT                   = 0x00000040, ///< SMT packets that an FDDI NIC receives.
  RNDIS_PACKET_TYPE_ALL_LOCAL             = 0x00000080, ///< All packets sent by installed protocols and all packets indicated by the NIC that is identified by a given NdisBindingHandle.
  RNDIS_PACKET_TYPE_GROUP                 = 0x00001000, ///< Packets sent to the current group address.
  RNDIS_PACKET_TYPE_ALL_FUNCTIONAL        = 0x00002000, ///< All functional address packets, not just the ones in the current functional address.
  RNDIS_PACKET_TYPE_FUNCTIONAL            = 0x00004000, ///< Functional address packets sent to addresses included in the current functional address.
  RNDIS_PACKET_TYPE_MAC_FRAME             = 0x00008000, ///< NIC driver frames that a Token Ring NIC receives.
  RNDIS_PACKET_TYPE_NO_LOCAL              = 0x00010000,
} rndis_packet_filter_type_t;

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CDC_RNDIS_H_ */

/** @} */
/** @} */


#endif /* Target checking */