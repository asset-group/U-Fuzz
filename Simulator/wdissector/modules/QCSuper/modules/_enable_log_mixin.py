#!/usr/bin/python3
#-*- encoding: Utf-8 -*-
from struct import pack, unpack_from, calcsize
from logging import warning, info
from protocol.log_types import *
from protocol.messages import *
from protocol.subsystems import ENABLED_SUBSYSTEMS
from time import sleep

"""
    This module exposes a class from which a module may inherit in order to
    enable logging packets.
"""

"""
    Definitions:

    * A "log" is a packet containing debugging information sent asynchronously
      through Diag by the baseband, as a raw binary structure, at the condition
      that the user has registered interest for this log or category of logs.

    * A "log code" is a 16-bit value referencing a specific binary structure
      that may be send through a log. Example: WCDMA_SIGNALLING_MESSAGE (0x412f)
      contains raw 3G signalling packets, with a small proprietary header.
    
    * A "log type" is the made up of the high 4-bits of the "log code", and
      encompass a broad range of logs. Example: WCDMA (0x4) for 3G-related logs.
    
    The user may register interest in log codes using a large bit mask, that
    is what this module does.
"""

LOG_CONFIG_RETRIEVE_ID_RANGES_OP = 1
LOG_CONFIG_SET_MASK_OP = 3

LOG_CONFIG_SUCCESS_S = 0

"""
    The following list enumerate log types used by the --pcap-dump and
    --json-geo-dump modules, and is used to restrict the quantity of logs
    that QCSuper registers to the baseband device (otherwise the volume of
    logs could be huge and it would be good to avoid potential performance
    impact on untested environment).

    Before you add a new start using a new log type in this module, be sure
    to add it to the list below.
"""

TYPES_FOR_RAW_PACKET_LOGGING = [
    
    # Layer 2:
    
    LOG_GPRS_MAC_SIGNALLING_MESSAGE_C, # 0x5226
    # LOG_LTE_MAC_DL_TRANSPORT_BLOCK_C,  # 0xb063
    # LOG_LTE_MAC_UL_TRANSPORT_BLOCK_C,  # 0xb064

    # Layer 3:
    
    LOG_GSM_RR_SIGNALING_MESSAGE_C, # 0x512f
    WCDMA_SIGNALLING_MESSAGE, # 0x412f
    LOG_LTE_RRC_OTA_MSG_LOG_C, # 0xb0c0
    LOG_NR_RRC_OTA_MSG_LOG_C, # 0xb821
    
    # NAS:
    
    LOG_UMTS_NAS_OTA_MESSAGE_LOG_PACKET_C, # 0x713a
    
    LOG_LTE_NAS_ESM_OTA_IN_MSG_LOG_C, # 0xb0e2
    LOG_LTE_NAS_ESM_OTA_OUT_MSG_LOG_C, # 0xb0e3
    LOG_LTE_NAS_EMM_OTA_IN_MSG_LOG_C, # 0xb0ec
    LOG_LTE_NAS_EMM_OTA_OUT_MSG_LOG_C, # 0xb0ed
    
    # User IP traffic:
    
    LOG_DATA_PROTOCOL_LOGGING_C # 0x11eb
]

class EnableLogMixin:

    log_messages = False
    
    def on_init(self):
        
        self.log_type_to_mask_bitsize = {}
        
        # Send the message for receiving the highest valid log code for
        # each existing log type (see defintions above).
        
        opcode, payload = self.diag_input.send_recv(DIAG_LOG_CONFIG_F, pack('<3xI', LOG_CONFIG_RETRIEVE_ID_RANGES_OP))
        
        header_spec = '<3xII'
        operation, status = unpack_from(header_spec, payload)
        
        assert operation == LOG_CONFIG_RETRIEVE_ID_RANGES_OP
        
        if status != LOG_CONFIG_SUCCESS_S:
            
            warning('Warning: log operation %d resulted in status %d' % (operation, status))
        
        log_masks = unpack_from('<16I', payload, calcsize(header_spec))
        
        # Iterate on information contained in the packet for each log type
        
        log_types = {
            0x1: '1X',
            0x4: 'WCDMA',
            0x5: 'GSM',
            0x6: 'LBS',
            0x7: 'UMTS',
            0x8: 'TDMA',
            0x9: 'Reserved09',
            0xA: 'DTV',
            0xB: 'APPS/LTE/WIMAX',
            0xC: 'DSP',
            0xD: 'TDSCDMA',
            0xF: 'TOOLS',
        }
        
        information_string = '[+] Enabled logging for: '
        
        for log_type, log_mask_bitsize in enumerate(log_masks):
            
            # Register logging for each supported log type
            
            if log_mask_bitsize:
                
                self.log_type_to_mask_bitsize[log_type] = log_mask_bitsize
                
                log_mask, logs_count = self._fill_log_mask(log_type, log_mask_bitsize)
                
                opcode, payload = self.diag_input.send_recv(DIAG_LOG_CONFIG_F, pack('<3xIII',
                    LOG_CONFIG_SET_MASK_OP,
                    log_type,
                    log_mask_bitsize
                ) + log_mask)

                operation, status = unpack_from(header_spec, payload)
                
                assert operation == LOG_CONFIG_SET_MASK_OP
                
                if status != LOG_CONFIG_SUCCESS_S:
                    
                    warning('Warning: log operation %d resulted in status %d' % (operation, status))
                
                information_string += '(%d) %s - Count=%d,\n' % (log_type, log_types.get(log_type, 'UNKNOWN'), logs_count)
        
        info('\n' + information_string.strip(',\n') + '\n')

        if self.log_messages:
            info('Enabling messages for enabled subsystems')
            for ssid_start, ssid_end in ENABLED_SUBSYSTEMS:
                # enable/disable messages for known subsystem
                self.send_msg_config_set_rt_mask(ssid_start, ssid_end, on = True)

    def send_msg_config_set_rt_mask(self, ssid_start, ssid_end, on = True):

        data = b''

        # create masks list for specified subsystems
        for i in range(0, ssid_end - ssid_start + 1):
            data += pack('<I', 0xffffffff if on else 0)

        # send request
        opcode, payload = self.diag_input.send_recv(DIAG_EXT_MSG_CONFIG_F,
                   pack('<BHHH', 0x04, ssid_start, ssid_end, 0) + data)

        # Opcode should return DIAG_EXT_MSG_CONFIG_F here

    def _fill_log_mask(self, log_type, num_bits, bit_value = 1):
        
        log_mask = b''
        logs_count = 0
        
        current_byte = 0
        num_bits_written = 0
        
        for i in range(num_bits):
            
            enable_this_log_type = True
            
            # limit_registered_logs: When set by a module inheriting this
            # class, this attribute may instruct to limit the logs to register
            # interest for a restricted set of log codes, in order to limit the
            # bulk of data sent from the device to the Diag client.
            
            if (hasattr(self, 'limit_registered_logs') and
                ((log_type << 12) | i) not in self.limit_registered_logs):
                
                enable_this_log_type = False
            
            current_byte |= (bit_value & enable_this_log_type) << num_bits_written
            num_bits_written += 1
            
            if num_bits_written == 8 or i == num_bits - 1:
                
                log_mask += bytes([current_byte])
                
                current_byte = 0
                num_bits_written = 0
            
            if enable_this_log_type:
                logs_count += 1

        return log_mask, logs_count
    
    def on_deinit(self):
        
        for log_type, log_mask_bitsize in getattr(self, 'log_type_to_mask_bitsize', {}).items():
            
            log_mask = self._fill_log_mask(log_type, log_mask_bitsize, bit_value = 0)

            if type(log_mask) == tuple:
                log_mask = log_mask[0]

            self.diag_input.send_recv(DIAG_LOG_CONFIG_F, pack('<3xIII',
                LOG_CONFIG_SET_MASK_OP,
                log_type,
                log_mask_bitsize
            ) + log_mask)

