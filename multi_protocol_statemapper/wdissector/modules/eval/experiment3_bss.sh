#/usr/bin/env bash

# Force correct relative path
cd "$(dirname ${BASH_SOURCE[0]:-$0})"
CURRENT_PATH="$(pwd)"

./flash_esp32.sh /dev/ttyUSB3

sudo apt install libbluetooth-dev -y

git clone https://github.com/joswr1ght/bss
cd bss
make

cd $HOME/esp-idf/
source export.sh
cd examples/bluetooth/bluedroid/classic_bt/bt_spp_acceptor 

idf.py -p /dev/ttyUSB3 monitor > bss_output.txt &

cd $CURRENT_PATH/bss

while true
do
	sudo ./bss -i hci0 -s 100 -m 12 -q 08:3A:F2:31:1C:B2
	sleep 1
done
