#!/usr/bin/env bash

BDTARGET=08:3a:f2:31:1c:b2
TARGETPORT=/dev/ttyUSB3

# Force correct relative path
cd "$(dirname ${BASH_SOURCE[0]:-$0})"
CURRENT_PATH="$(pwd)"
cd ../../
# backup original model
if [[ ! -f configs/models/bt/sdp_rfcomm_query.json ]]
then
    cp configs/models/bt/sdp_rfcomm_query.json configs/models/bt/sdp_rfcomm_query_original.json
fi

# Generate all reference models
cd modules/eval
./gen_models.py

cd $CURRENT_PATH/../../
# Evaluate variant 'eval_1'
cp modules/eval/refs/1.json configs/models/bt/sdp_rfcomm_query.json -f
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=true --optimization=true
rm modules/eval/eval_1 -rdf || true && cp logs/Bluetooth modules/eval/eval_1 -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'eval_15'
cp modules/eval/refs/15.json configs/models/bt/sdp_rfcomm_query.json -f
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=true --optimization=true
rm modules/eval/eval_15 -rdf || true && cp logs/Bluetooth modules/eval/eval_15 -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'eval_30'
cp modules/eval/refs/30.json configs/models/bt/sdp_rfcomm_query.json -f
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=true --optimization=true
rm modules/eval/eval_30 -rdf || true && cp logs/Bluetooth modules/eval/eval_30 -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'eval_45'
cp modules/eval/refs/45.json configs/models/bt/sdp_rfcomm_query.json -f
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=true --optimization=true
rm modules/eval/eval_45 -rdf || true && cp logs/Bluetooth modules/eval/eval_45 -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2
# Evaluate variant 'eval_60'
cp modules/eval/refs/60.json configs/models/bt/sdp_rfcomm_query.json -f
./modules/eval/flash_esp32.sh $TARGETPORT # Flash ESP32 firmware
sudo bin/bt_fuzzer --no-gui --target=$BDTARGET --duplication=true --mutation=true --optimization=true
rm modules/eval/eval_60 -rdf || true && cp logs/Bluetooth modules/eval/eval_60 -r
sleep 2 && sudo rm logs/Bluetooth -rdf && sleep 2


# Analyse resulting logs
cd modules/eval
./gen_models_plot.py
echo -e "Experiment 2 completed, check the output and graph_models.pdf"
