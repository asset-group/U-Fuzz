#/usr/bin/env bash

# Force correct relative path
cd "$(dirname ${BASH_SOURCE[0]:-$0})"
CURRENT_PATH="$(pwd)"

sudo apt-get install bluetooth -y
sudo apt-get install bluez -y
sudo apt-get install python-bluez -y
sudo apt-get install libbluetooth-dev bluez-hcidump  libboost-python-dev libboost-thread-dev libglib2.0-dev -y
sudo apt-get install tshark -y
sudo pip install pybluez gattlib

git clone https://github.com/lucaboni92/BlueFuzz

cd $HOME/esp-idf/
source export.sh
cd examples/bluetooth/bluedroid/classic_bt/bt_spp_acceptor 

idf.py -p /dev/ttyUSB3 monitor > bluefuzz_output.txt &

cd $CURRENT_PATH/BlueFuzz


while true
do
	sudo python bluetooth_scanner.py
	sleep 1
done