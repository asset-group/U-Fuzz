#!/usr/bin/env bash

# usage: flash_esp32.sh /dev/ttyUSB3

cd $HOME/esp-idf/
source export.sh
cd examples/bluetooth/bluedroid/classic_bt/bt_spp_acceptor 
idf.py build
idf.py -p $1 erase_flash flash