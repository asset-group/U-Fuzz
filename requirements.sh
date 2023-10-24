#!/usr/bin/env bash


if [ "$1" == "dev" ]
then
	# Ubuntu dev. requirements
	sudo apt-get install lsb-release -y

	UBUNTU_VER=$(lsb_release -rs)

	sudo apt install software-properties-common gzip curl git wget zstd python3-dev flex bison pkg-config swig graphviz libglib2.0-dev libgcrypt-dev \
	libgraphviz-dev liblz4-dev libsnappy-dev libgnutls28-dev libxml2-dev libnghttp2-dev libkrb5-dev libnl-3-dev libspandsp-dev libxrandr-dev libxinerama-dev libxcursor-dev \
	libxi-dev libnl-genl-3-dev libnl-route-3-dev libnl-nf-3-dev libcap-dev libbrotli-dev libsmi-dev libc-ares-dev qt5-default qttools5-dev qttools5-dev-tools libsbc-dev \
	libqt5svg5-dev qtmultimedia5-dev libspeexdsp-dev libfreetype6-dev libxss1 libtbb-dev libnss3-dev libudev-dev libpulse-dev libpcre2-dev \
	libasound2-dev psmisc sshpass -y
	# Open5GS requirements
	sudo apt install -y libtalloc-dev libconfig-dev build-essential libsctp-dev libgcrypt-dev libssl-dev libidn11-dev libmongoc-dev libbson-dev libyaml-dev \
	libmicrohttpd-dev libcurl4-gnutls-dev autoconf libpcap-dev libtool mongodb
	# Wi-Fi requirements
	# sudo apt install linux-headers-$(uname -r) -y
	sudo apt install dnsmasq net-tools iptables -y
	sudo cp src/drivers/wifi/rtl8812au/85-nm-unmanaged.rules /etc/udev/rules.d/85-nm-unmanaged.rules
	sudo udevadm control --reload-rules && udevadm trigger

	# install clang-11 for Ubuntu
	sudo ./scripts/install_llvm.sh 11

	# Install nodejs
	if ! which node > /dev/null;
	then
		echo "nodejs not found, installing now..."
		curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
		sudo apt update
		sudo apt install nodejs -y
	else
		echo "nodejs found!"
	fi

	# Install quicktype
	if ! which quicktype > /dev/null; 
	then
	echo "quicktype not found, installing now..."
	sudo npm install -g quicktype || echo -e "\nError. Make sure to install nodejs"
	else
	echo "quicktype found!"
	fi

	# Install python pip
	if ! which pip3 > /dev/null;
	then
	echo "pip3 not found, installing now..."
	wget https://bootstrap.pypa.io/pip/3.6/get-pip.py
	sudo python3 get-pip.py
	rm get-pip.py
	else
	echo "pip3 found!"
	fi

	# Python3 packages
	sudo python3 -m pip install \
	ctypesgen \
	cmake==3.18.4 -U \
	ninja==1.10.0 -U \
	meson==0.50.1

	# Evaluation packages
	sudo python3 -m pip install numpy pandas python-pcapng==1.0 matplotlib
	sudo apt -y install expect-dev


	# Download 3rd-party repos submodules and apply patch
	# ./scripts/apply_patches.sh

elif [ "$1" == "doc" ]
then
	sudo apt install python2 build-essential -y

	# Install nodejs
	if ! which node > /dev/null;
	then
		echo "nodejs not found, installing now..."
		curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
		sudo apt update
		sudo apt install nodejs -y
	else
		echo "nodejs found!"
	fi
	# Install local nodejs documentation requirements
	npm install
	cd docs/old_greyhound/
	npm install
	cd ../../
else
	# Minimal Ubuntu Packages to run binary WDissector and Wireshark distribution
	sudo apt install g++ libglib2.0-dev qt5-default libqt5multimedia5 libsnappy1v5 libsmi2ldbl libc-ares2 libnl-route-3-200 \
	libfreetype6 graphviz libtbb-dev libxss1 libnss3 libspandsp2 libsbc1 libbrotli1 libnghttp2-14 libasound2 psmisc sshpass \
	libpulse0 libasound2 libpcre2-dev -y
	# Wi-Fi requirements
	sudo apt install dnsmasq net-tools iptables linux-headers-$(uname -r) -y
	sudo cp src/drivers/wifi/rtl8812au/85-nm-unmanaged.rules /etc/udev/rules.d/85-nm-unmanaged.rules
	sudo udevadm control --reload-rules && udevadm trigger
	cd src/drivers/wifi/rtl8812au/
	make -j4

	# Evaluation packages
	sudo python3 -m pip install numpy pandas python-pcapng==1.0 matplotlib
	sudo apt install expect-dev
fi

# TODO: Check requirement of the following runtime libraries
# libbrotli1
# libnghttp2-14 
# libspeexdsp1
# libnl-genl-3-200
# libasound2
