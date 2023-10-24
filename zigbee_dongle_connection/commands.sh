#!/usr/bin/env bash

sudo apt install -y mosquitto
sudo systemctl stop mosquitto
sudo systemctl disable mosquitto

mosquitto