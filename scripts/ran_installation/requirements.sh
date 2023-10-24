#!/usr/bin/env bash


sudo apt update
sudo apt install wget curl libtool autoconf mongodb libpcap-dev software-properties-common -y
sudo apt install python3-pip python3-setuptools python3-wheel libconfig-dev \
ninja-build build-essential flex bison git libsctp-dev libgnutls28-dev \
libgcrypt-dev libssl-dev libidn11-dev libmongoc-dev libbson-dev libyaml-dev \
libnghttp2-dev libmicrohttpd-dev libcurl4-gnutls-dev libnghttp2-dev meson -y

sudo python3 -m pip install ninja cmake==3.18.4 meson==0.50.1 -U

# Install nodejs if npm is not found
if ! which npm > /dev/null;
then
	echo "nodejs not found, installing now..."
	curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
	sudo apt update
	sudo apt install nodejs -y
else
	echo "nodejs found!"
fi

# Install latest stable wireshark
sudo add-apt-repository ppa:wireshark-dev/stable -y
sudo apt-get update
sudo apt install wireshark-dev -y

# -------------------- Download WDissector -----------------------------
echo "Downloading WDissector binary release"
wget --header="PRIVATE-TOKEN: ZSCBFGGnsvj9J8eBdhs9" \
--content-disposition "https://gitlab.com/api/v4/projects/\
asset-sutd%2Fsoftware%2Fwireless-deep-fuzzer/jobs/artifacts/\
wdissector/raw/release/wdissector.tar.zst?job=release"
# Install zstandard
sudo apt install zstd -y
# Extract the wdissector compressed file
tar -I zstd -xf wdissector.tar.zst
rm wdissector.tar.zst
cd wdissector
./requirements.sh
cd ../
