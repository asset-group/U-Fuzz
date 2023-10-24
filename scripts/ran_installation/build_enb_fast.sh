#!/usr/bin/env bash

cd oai_lte_network/openair-ran/cmake_targets/lte_build_oai/build/
ninja lte-softmodem

# stop ModemManager
# sudo service ModemManager stop
# sudo systemctl disable ModemManager.service