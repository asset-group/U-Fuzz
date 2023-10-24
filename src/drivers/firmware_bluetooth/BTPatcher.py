from ELFPatcher import XtensaPatcher


def patch_esp32(elf_path):
    print('-------------------------------- BT Patch -----------------------------------------')
    ESP32Patch = XtensaPatcher(elf_path, always_backup=True)
    # Patch ACL Control TX (This is patched during runtime instead)
    # ESP32Patch.ApplyHook('r_lc_send_lmp+53', 'hook_tx_acl_control')  # Patch memcopy
    # ESP32Patch.ApplyHook('r_lc_send_lmp+85', 'reversed_r_ld_acl_lmp_tx')  # Patch tx path
    # Patch ACL Data TX
    ESP32Patch.ApplyInst('r_ld_acl_data_tx+605', 'mov.n', 'a14', 'a3')
    ESP32Patch.ApplyInst('r_ld_acl_data_tx+607', 'mov.n', 'a15', 'a1')
    ESP32Patch.ApplyInst('r_ld_acl_data_tx+609', 'call12', 'hook_tx_acl_data')
    # ------------------- ACL RX Path ------------------------
    # Hook to sniff BT RX Packets
    ESP32Patch.ApplyInst('r_ld_fm_frame_isr+70', 'call8',
                         'hook_frame_isr')
    # Hook to accept or reject BT ISR
    ESP32Patch.ApplyInst('r_ld_fm_frame_isr+318', 'call8',
                         'hook_frame_isr_callback')
    # Custom Hook function
    ESP32Patch.ApplyInst('config_lc_task_funcs_reset+3', 'call8',
                         'custom_lc_reset_lc_default_state_funcs')
    # ----------------------- Misc ---------------------------
    # LOGs Disabling Patch
    # Disable warning when receiving unsolicited responses from target
    ESP32Patch.ApplyInst('lc_lmp_rx_handler+125', 'nop')
    print('-----------------------------------------------------------------------------------')


if __name__ == "__main__":
    patch_esp32('.pio/build/esp32doit-devkit-v1/firmware.elf')
