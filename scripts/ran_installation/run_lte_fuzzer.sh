#!/usr/bin/env bash

cd wdissector

sudo kill -9 $(pgrep lte-softmodem*)
sudo bin/lte_fuzzer