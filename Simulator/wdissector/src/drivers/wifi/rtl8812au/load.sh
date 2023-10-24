#!/usr/bin/env bash

cd $(dirname ${BASH_SOURCE[0]:-$0})

# make -j && sudo rmmod 88XXau
echo "Removing driver if loaded..."
sudo rmmod 88XXau &> /dev/null
echo "Loading driver now..."
sudo insmod 88XXau.ko rtw_monitor_retransmit=1 rtw_monitor_disable_1m=1 rtw_drv_log_level=3 rtw_led_ctrl=1 rtw_vht_enable=1 rtw_power_mgnt=0 rtw_switch_usb_mode=2 rtw_beamform_cap=0 rtw_default_beacon_interval=100
