#!/usr/bin/env bash

BDTARGET=08:3a:f2:31:1c:b2
TARGETPORT=/dev/ttyUSB3

# Force correct relative path
cd "$(dirname ${BASH_SOURCE[0]:-$0})"
cd ../../

# Evaluate variant 'all'
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=true --optimization=true
rm modules/eval/all -rdf || true && cp logs/Bluetooth modules/eval/all -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'dup'
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=false --optimization=false
rm modules/eval/dup -rdf || true && cp logs/Bluetooth modules/eval/dup -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'mut'
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=false --mutation=true --optimization=false
rm modules/eval/mut -rdf || true && cp logs/Bluetooth modules/eval/mut -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'evo'
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=false --mutation=true --optimization=true
rm modules/eval/evo -rdf || true && cp logs/Bluetooth modules/eval/evo -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2

# Analyse resulting logs
cd modules/eval
./eval.py all
./eval.py dup
./eval.py mut
./eval.py evo
./gen_plot.py
echo -e "Experiment 1 completed, check the output and graph_optimization.pdf"
