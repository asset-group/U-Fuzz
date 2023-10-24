#!/usr/bin/env bash

git clone https://gitlab.com/asset-sutd/software/oai_lte_network --depth=1
cd oai_lte_network/openair-ran
# Requirements will be installed to the system
./build_all.sh
# ./build.sh

# stop ModemManager
# sudo service ModemManager stop
# sudo systemctl disable ModemManager.service
