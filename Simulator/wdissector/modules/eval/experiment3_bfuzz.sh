#/usr/bin/env bash

# Force correct relative path
cd "$(dirname ${BASH_SOURCE[0]:-$0})"
CURRENT_PATH="$(pwd)"

wget https://iotcube.net/tools/bf1/bfuzz.zip
unzip bfuzz

cd $HOME/esp-idf/
source export.sh
cd examples/bluetooth/bluedroid/classic_bt/bt_spp_acceptor 

idf.py -p /dev/ttyUSB3 monitor > bfuzz_output.txt &

cd $CURRENT_PATH/bfuzz
sudo ./config_bfuzz.sh
sudo ./bfuzz
