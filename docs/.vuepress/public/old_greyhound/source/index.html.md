---
title: Greyhound
includes: []
search: true
highlight_theme: darkula
headingLevel: 2
toc_footers:
  - >-
    <a href="app/index.html">Greyhound Web GUI</a>
  - >-
    <a href="https://www.keysight.com/sg/en/home.html" target="_blank"><img src="source/images/keysight_logo_white.svg" height="auto" width="88%"></a>
  - >-
    <a href="https://sutd.edu.sg/" target="_blank"><img style="padding-left:18px" src="source/images/sutd-logo-small.png" height="auto" width="70%"></a>
    
---

# Greyhound

> Greyhound folder structure
> ![g_folders](source/images/greyhound_folders.svg)

Greyhound is a greybox directed protocol fuzzing tool. Currently, only support for Wi-Fi clients is added, which includes WPA2-Personal and WPA2-Enterprise testing. Support for for Bluetooth Low Energy 4.2 is undergoing.

The main idea of GreyHound is to leverage a stateful wireless protocol model in order to find vulnerabilities in other wireless device by means of fuzzing. Packets are are sent to the tested wireless device during such states, but are sometimes fuzzed based on a probability array. This array, in turn, is improved using PSO algorithm in order to cover different parts of the target wireless implementation. Later on, GreyHound also verifies the wireless device against anomalies by checking its response against a set of expected packets previously defined for the specific wireless protocol being tested. 

![greyhound_arch](source/images/greyhound_arch.png)

GreyHound files are structured as follows:
* **Wireless models** - **wifi_ap.py** refers to a Wi-Fi access point while **ble_central.py** emulates a BLE Central device.
* **Core files** - Files within **greyhound** folder. Functionalities such as fuzzing, validation and cost functions are here.
* **Monitors** - The monitoring component is responsible to detect crashes via different methods. Currently, only serial monitor is available. External monitors can be added by using [Greyhound Web API](#----web-api).
* **Drivers** - The driver folder contains custom code that enables GreyHound wireless models to communicate with the intended radio hardware.
* **Logs** - The **logs** folder store Wireshark captures and reports of possible anomalies.
* **Auxiliary files** - Auxiliary code files contain models specific code.
* **Optimization** - **bench.py** is the code that performs fuzzing optimization. [Pagmo2](https://esa.github.io/pagmo2/) is used as the optimization algorithms framework. The cost function is selected here. 

# :wrench: ​Installation

## Requirements

In order to use Greyhound, it's recommended to use Pypy (Python alternative interpreter) and some other Linux packages. If you are using a Debian based distribution such as **Ubuntu** or **Kali**, you can use apt package manager.

> Shell - Install linux packages
```bash
sudo apt-get install build-essential linux-headers-$(uname -r) python \
python-pip pypy pypy-dev wget git aircrack-ng \
wireless-tools freeradius graphviz \
libgraphviz-dev pkg-config python-matplotlib libssl-dev

wget https://bootstrap.pypa.io/get-pip.py
sudo pypy get-pip.py
rm get-pip.py
sudo apt-get install --reinstall python-pip
```

The following packages are required:
* Kernel Source (For building the custom Wi-Fi driver);
* build-essential
* python 
* python-pip
* pypy
* pypy-dev
* aircrack-ng
* wireless-tools
* freeradius
* graphviz 
* libgraphviz-dev
* pkg-config
* python-matplotlib
* libssl-dev
* wget



When installing pip for Pypy, the default `pip` command will point to `Pypy-pip`, not `Python2.7-pip`, thus the command `sudo apt-get install --reinstall python-pip` fixes that.

After all the required packages are installed, few more steps must be performed:
> Shell - Install python libraries
```bash
cd WirelessDeepFuzzer
sudo pypy -m pip install -r requirements.txt
python -m pip install -r requirements2.txt
pypy bluetooth/ble_server/setup.py build install
cd wifi/eap_module
make
make install
cd ../../
```

1. Install pypy and python requirements in files `requirements.txt` and `requirements2.txt` respectively.
2. Compile and install the **SMPServer** library in folder `bluetooth/smp_server`.
3. Compile and install the **EAPModule** library in folder `wifi/eap_module`.

<aside>If you have permission, you can git pull or download Greyhound repository via the link <a href="https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/tree/develop/WirelessDeepFuzzer">https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/tree/develop/WirelessDeepFuzzer</a></aside>
.

## Custom Drivers

GreyHound uses custom drivers to receive or send raw packets through the air. Follow the steps described in Wi-Fi and BLE to use such drivers for the supported wireless models.

### Wi-Fi driver

> Shell - Compile and install Wi-Fi driver​
```python
cd drivers/RT2800/
mkdir ../backup
cp /usr/lib/modules/$(uname -r)/kernel/drivers/net/wireless/ralink/rt2x00/* ../backup/
sudo make all install insert
cd ../../ # Get back to geryhound folder
sudo cp wifi/freeradius_config/* /etc/freeradius/3.0/ -R # Update freeradius config
```

For Wi-Fi, a Ralink 2080USB driver was modified to allow real time communication with the Wi-Fi hardware, bypassing the Linux networking stack. The driver **rt2800usb.py** allows free configuration of any hardware register documented in [Ralink datasheet](http://www.lan23.ru/wifi/EEPROM-WN7200ND/RT3070.pdf). While there's many parameters to configure, GreyHound uses the default options and only configures packet filtering and automatic acknowledgements response. The last option is necessary to prevent the tested Wi-Fi devices from dropping packets received from GreyHound.

To install this driver, it's necessary to compile it against your kernel version. The installation overwrites the original RT2800 drivers, thus, it's recommended to perform a backup of the original driver from your kernel modules folder first. If the module is correctly compiled, running `dmesg` on terminal will output "Netlink" messages. The log will show the number of the registered Netlink device (Device 7 in the example).


> Shell - dmesg output (Kernel log)
```shell
[May27 15:38] RT2800USB: Service started
[  +0.000028] usbcore: registered new interface driver rt2800usb
[  +0.000005] Netlink: Started on port 31.
[May27 15:39] usb 1-8: new high-speed USB device number 7 using xhci_hcd
[  +0.169372] usb 1-8: New USB device found, idVendor=148f, idProduct=3070, bcdDevice= 1.01
[  +0.000002] usb 1-8: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[  +0.000001] usb 1-8: Product: 802.11 n WLAN
[  +0.000000] usb 1-8: Manufacturer: Ralink
[  +0.000001] usb 1-8: SerialNumber: 1.0
[  +0.134999] usb 1-8: reset high-speed USB device number 7 using xhci_hcd
[  +0.178756] ieee80211 phy1: Selected rate control algorithm 'minstrel_ht'
[  +0.022692] IPv6: ADDRCONF(NETDEV_UP): wlan1: link is not ready
[  +0.000046] ieee80211 phy1: rt2x00lib_request_firmware: Infos - Loading firmware file 'rt2870.bin'
[  +0.000237] rt2800usb 1-8:1.0: firmware: direct-loading firmware rt2870.bin
[  +0.000004] ieee80211 phy1: rt2x00lib_request_firmware: Infos - Firmware detected - version: 0.36
[  +0.055699] Netlink: Device 7 registered to server
```

The figure below shows the components of the custom driver. Note that several network components are bypassed by GreyHound patches, allowing direct reception and transmission of packets from user space to kernel by using a Netlink socket (steps 1 and 2).

![wifi_custom_driver](source/images/wifi_custom_driver.png)

### Bluetooth Low Energy driver

> Shell - Flashing pre-built BLE driver firmware
```bash
# Flash s140_nrf52_6.1.1_softdevice.hex with nRF Connect first
pip install nrfutil # install nrfutil flashing tool
sudo nrfutil dfu usb-serial -p /dev/ttyACM0 -pkg drivers/NRF52_Dongle/app_dfu_package.zip
# Change /dev/ttyACM0 if necessary
```

The Bluetooth Low Energy driver uses [Nordic nRF52840 Dongle](https://www.nordicsemi.com/?sc_itemid=%7BCDCCA013-FE4C-4655-B20C-1557AB6568C9%7D) to forward raw link layer packet to and from Greyhound. It's necessary to flash the driver firmware to the board before using the BLE model.

**Before any step, Nordic softdevice NRF52 s140 firmware needs to be flashed first. You can get this file [here](source/files/s140_nrf52_6.1.1_softdevice.hex) or by downloading [Nordic 15.3.0 nRF5 SDK](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK/Download#infotabs). This file can be flashed by using [nRF Connect for Desktop](https://www.nordicsemi.com/?sc_itemid=%7BB935528E-8BFA-42D9-8BB5-83E2A5E1FF5C%7D). You also need to put the board in programming mode by pressing the reset button on the board.**

![prog_m](source/images/nRF52840_Dongle.png)

To quickly use this driver, the firmware binary is already provided in **[drivers/NRF52_Dongle/app_dfu_package.zip](source/files/app_dfu_package.zip)**
> Shell - Flash from sources (optional)
```Python
pip install -U platformio
cd drivers/NRF52_Dongle
platformio run # platformio will download toolchains and SDKs
```
> Output - Compile from sources (optional)
> ![platformio_run](source/images/platformio_run.png)

```Python
# Prepare firmware package
nrfutil pkg generate --hw-version 52 --application-version 1 --application .pioenvs/adafruit_feather_nrf52840/firmware.hex --sd-req 0xB6 app_dfu_package.zip
# Put board in program mode first (press the reset)
sudo nrfutil dfu usb-serial -p /dev/ttyACM0 -pkg app_dfu_package.zip # Flash built firmware
```

If you want to build from sources, perform the following steps:
* Install [PlatformIO Core](https://docs.platformio.org/en/latest/installation.html) to your system;
* Put the NRF52 in programming mode. You should see a red LED blinking.
* Flash the firmware by using the provided platformio and nrfutil commands.
* After the procedure, a green LED should blink intermittently.

This firmware allows raw injection/reception of BLE link layer frames. The NRF52 USB board communicates with Greyhound via a protocol handled by python module **drivers/NRF52_Dongle.py**. Currently, only the initiator role is implemented.

![ble_driver](source/images/ble_driver.png)



## Raspberry Pi 3 installation

> Shell - Raspberry Pi 3 installation

```bash
sudo apt-get update
sudo apt-get install build-essential raspberrypi-kernel-headers python \
python-pip pypy pypy-dev wget git aircrack-ng cmake \
wireless-tools freeradius graphviz libgraphviz-dev \
pkg-config python-matplotlib libssl-dev libboost1.62-dev \
libboost-serialization1.62-dev libboost-python1.62-dev flex \
bc bison libffi-dev
# Install pypy and python libraries
sudo pypy -m pip install setuptools # Requiredby cryptography
sudo pypy -m pip install cryptography
sudo pypy -m pip install -r WirelessDeepFuzzer/requirements.txt
sudo python -m pip install cloudpickle socketIO_client flask_socketio pycallgraph socketio
pypy WirelessDeepFuzzer/bluetooth/ble_server/setup.py build install

# Compile and install custom Wi-Fi Driver
cd $HOME
sudo modprobe configs # generate current kernel config
git clone https://github.com/raspberrypi/linux --branch raspberrypi-kernel_1.20190709-1 --depth=1
sudo ln -s /home/pi/linux /lib/modules/$(uname -r)/build
sudo ln -s /home/pi/linux /lib/modules/$(uname -r)/source
zcat /proc/config.gz > linux/.config
cd linux && make linux modules_prepare
# Apply patches from rt2800 rpi drivers
sudo cp drivers/net/wireless/ralink/rt2x00/rt2x00pci.c $HOME/WirelessDeepFuzzer/drivers/RT2800/rt2x00pci.c
sudo cp drivers/net/wireless/ralink/rt2x00/rt2x00pci.h $HOME/WirelessDeepFuzzer/drivers/RT2800/rt2x00pci.h
sudo cp drivers/net/wireless/ralink/rt2x00/rt2800pci.c $HOME/WirelessDeepFuzzer/drivers/RT2800/rt2800pci.c
sudo cp drivers/net/wireless/ralink/rt2x00/rt2800pci.h $HOME/WirelessDeepFuzzer/drivers/RT2800/rt2800pci.h
# Compile and install Wi-Fi driver
cd WirelessDeepFuzzer/drivers/RT2800
sudo make all install insert PWD=$HOME/WirelessDeepFuzzer/drivers/RT2800
sudo iwlist wlan1 scan # Test and initialize driver (insert dongle)

# Get libtbb requirement for Pagmo2 (Optimization library)
git clone https://github.com/abhiTronix/TBB_Raspberry_pi --depth=1
sudo dpkg -i TBB_Raspberry_pi/libtbb-dev_4.5-1_armhf.deb # libtbb for rasp
sudo ldconfig
# Apply armv7 patch (gcc_armv7.h.patch)
patch /usr/local/include/tbb/machine/gcc_armv7.h gcc_armv7.h.patch
# Build and install Pagmo2
git clone https://github.com/esa/pagmo2.git --depth=1
# Apply pagmo2 cmake -latomic patch
patch pagmo2/CMakeLists.txt CMakeLists.txt.patch
cd pagmo2
mkir build
cd build
cmake ../
cmake --build .
sudo make install # Install /usr/local/lib/libpagmo.so
sudo ldconfig && rm CMakeCache.txt
# Build and install Pygmo
cmake ../ -DPAGMO_BUILD_PAGMO=OFF -DPAGMO_BUILD_PYGMO=ON
cmake --build .
sudo make install
sudo ldconfig
```

> Patch file - **gcc_armv7.h.patch**

```diff
--- /usr/local/include/tbb/machine/gcc_armv7.h	2019-09-15 10:00:45.827230020 +0100
+++ /usr/local/include/tbb/machine/gcc_armv7.h	2019-09-15 09:58:38.448122500 +0100
@@ -27,9 +27,9 @@
 #endif
 
 //TODO: is ARMv7 is the only version ever to support?
-#if !(__ARM_ARCH_7A__)
-#error compilation requires an ARMv7-a architecture.
-#endif
+//#if !(__ARM_ARCH_7A__)
+//#error compilation requires an ARMv7-a architecture.
+//#endif
 
 #include <sys/param.h>
 #include <unistd.h>
@@ -53,7 +53,7 @@
 
 
 #define __TBB_compiler_fence()    __asm__ __volatile__("": : :"memory")
-#define __TBB_full_memory_fence() __asm__ __volatile__("dmb ish": : :"memory")
+#define __TBB_full_memory_fence() 0xffff0fa0 // __asm__ __volatile__("dmb ish": : :"memory")
 #define __TBB_control_consistency_helper() __TBB_full_memory_fence()
 #define __TBB_acquire_consistency_helper() __TBB_full_memory_fence()
 #define __TBB_release_consistency_helper() __TBB_full_memory_fence()
```

> Patch file - **CMakeLists.txt.patch**

```diff
--- pagmo2/CMakeLists.txt	2019-09-15 13:05:48.000000000 +0100
+++ pagmo2/patch_CMakeLists.txt	2019-09-15 13:11:38.000000000 +0100
@@ -79,8 +79,8 @@
 include(YACMAThreadingSetup)
 
 # Assemble the flags.
-set(PAGMO_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG} ${YACMA_THREADING_CXX_FLAGS})
-set(PAGMO_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS} ${YACMA_THREADING_CXX_FLAGS})
+set(PAGMO_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG} ${YACMA_THREADING_CXX_FLAGS} -latomic)
+set(PAGMO_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS} ${YACMA_THREADING_CXX_FLAGS} -latomic)
 if(APPLE AND YACMA_COMPILER_IS_CLANGXX)
   message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
   list(APPEND PAGMO_CXX_FLAGS_DEBUG "-stdlib=libc++")
@@ -311,6 +311,8 @@
     # TBB.
     target_link_libraries(pagmo PUBLIC TBB::tbb)
 
+    target_link_libraries(pagmo PUBLIC -latomic)
+
     if(PAGMO_WITH_EIGEN3)
         # Link pagmo to eigen3.
         target_link_libraries(pagmo PUBLIC Eigen3::eigen3)
```


The advantage of using greyhound in a standalone device such as Raspberry Pi 3 is that a VM and a costly computer is not required. For these reasons, the steps required to run Greyhound in such platform is provided here.

There's some caveats when installing dependencies in ARM platforms, as such, some of these will need to be compiled from source as they are not provided in binaries. Furthermore, few patches need to be applied during the steps to ensure compatibility with Raspberry Pi.

The libraries which require more attention are:

* **libboost and libboost-serialization** - PyGMO requires libboost and libboost-serialization to be installed in the machine. It's possible to install such dependencies by simply using the apt package manager on Raspberry Pi
* **Intel TBB** - This library is required by pagmo2 and usually needs to be compiled from sources in most ARM platforms. Fortunately, as Raspberry Pi is mainstream, other users had already generated installation .deb packages for this library. However, a patch needs to be applied its headers for Pagmo to compile without errors. This patch Patches/CMakeLists.txt.patch solves this problem
* **pagmo2** - This library contains the shared library for PyGMO, however, it's cmake configuration doesn't add -latomic automatically, which prevent it to work correctly in ARM when importing pygmo python library. To solve this issue, the patch Patches/CMakeLists.txt.patch is provided.

Greyhound was tested in the lite version of raspbian ([2019-07-10-raspbian-buster-lite](http://director.downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2019-07-12/2019-07-10-raspbian-buster-lite.zip)). Assuming that your raspberry already contains the **WirelessDeepFuzzer** in the home folder, run the required steps in the terminal. It may take some hours to finish everything. 

When following the steps regarding the custom Wi-Fi compilation, make sure the dongle is plugged, so the command **sudo iwlist wlan1 scan** works.

<aside class="warning">Currently, Bluetooth Low Energy model has some problems in the BLESMPServer module which causes segmentation fault in Raspberry Pi during interaction with the peripheral. It's also recommended to use the latest Raspberry Pi 3 B+ or greater for a better execution speed</aside>
.




# :repeat_one: ​Wireless models

All the wireless models use the [PyTransitions](https://github.com/pytransitions/transitions) library in order to provide easy state machine construction and graph generation. In short, a model is a class which initialises Greyhound custom state machine.

> Python - Greyhound state machine object

```python
self.machine = GreyhoundStateMachine(states=machine_states,
                                     transitions=machine_transitions,
                                     print_transitions=True,
                                     print_timeout=True,
                                     initial='INIT',
                                     idle_state='WAIT_AUTH_REQUEST',
                                     show_conditions=False,
                                     show_state_attributes=False,
                                     enable_webserver=True)
```

| Parameter                 | Type       | Description                                                  |
| ------------------------- | ---------- | ------------------------------------------------------------ |
| **states**                | Dictionary | [Set of states](https://github.com/pytransitions/transitions#states). Each state may contain timeout information |
| **transitions**           | Dictionary | [Set of transitions](https://github.com/pytransitions/transitions#transitions) matching defined states |
| **print_transitions**     | Boolean    | Print information of a triggered transition                  |
| **print_timeout**         | Boolean    | Print information of a state timeout                         |
| **initial**               | String     | String defining the name of the initial state                |
| **idle_state**            | String     | String defining the idle state                               |
| **show_conditions**       | Boolean    | Show conditions in the model graph                           |
| **show_state_attributes** | Boolean    | Show state attributes in the model graph                     |
| **enable_webserver**      | Boolean    | Enable Greyhound [Web API](#web-api)                         |



## Wi-Fi AP model

The figure below provides a high-level overview of the state mahine model used by Greyhound. The model captures the core design of IEEE 802.11, 802.11i and 802.1X, which enables Greyhound to test implementations of both WPA2-Personal and WPA2-Enterprise against possible undesirable Wi-Fi client behaviours, such as non-compliance with the standard.

> Wi-Fi Stack model

> ![ble_stack](source/images/wifi_stack.png)

![wifi_ap_model](source/images/wifi_ap_model.png)

The model configuration can be changed in the json file **wifi_ap_config.json**. The following default configurations are applied:

> JSON - wifi_ap_config.json
```json
{
    "SSID": "TEST_KRA", 
    "Password": "testtest", 
    "Username": "matheus_garbelini", 
    "EAP": true, 
    "Channel": 12, 
    "MAC": "28:c6:3f:a8:af:c5", 
    "GatewayIP": "192.168.42.1", 
    "CustomDriverName": "RT2800USB", 
    "FuzzingInterface": "wlan0mon", 
    "ShareInternet": true, 
    "ShareInterface": "eth0", 
    "SerialPortName": "/dev/ttyUSB*", 
    "SerialPortBaud": "115200", 
    "CrashDetectionWord": "WPA2 ENTERPRISE VERSION"
}
```

| Parameter              | Default Value           | Description                                                  |
| :--------------------- | :---------------------- | ------------------------------------------------------------ |
| **SSID**               | TEST_KRA                | AP Network name                                              |
| **Password**           | testtest                | AP Network password                                          |
| **Username**           | matheus_garbelini       | Username if EAP enabled                                      |
| **EAP**                | true                    | Enable EAP (Enterprise)                                      |
| **Channel**            | 12                      | Wi-Fi Channel                                                |
| **MAC**                | 28:c6:3f:a8:af:c5       | Model WI-Fi MAC                                              |
| **GatewayIP**          | 192.168.42.1            | Default gateway IP address                                   |
| **CustomDriverName**   | RT2800USB               | Driver name used by Wi-Fi model                              |
| **FuzzingInterface**   | wlan0mon                | Wi-Fi interface used by Wi-Fi model                          |
| **ShareInternet**      | true                    | Enable internet sharing with a Wi-Fi client connected  to this model |
| **ShareInterface**     | eth0                    | Interface for internet sharing                               |
| **SerialPortName**     | /dev/ttyUSB*            | Serial monitor port for crash detection. Use `*` as wildcard. |
| **SerialPortBaud**     | 115200                  | Serial baudrate.                                             |
| **CrashDetectionWord** | WPA2 ENTERPRISE VERSION | Magic string used for crash detection in the monitored device |

> Python - Wi-Fi model initialization
```python
wifi_machine = Dot11Methods(states, transitions,
                            machine_show_all_transitions=False,
                            idle_state='WAIT_AUTH_REQUEST')
wifi_machine.setup_eap_transitions(wifi_machine.eap_enable) # Configure EAP
# Save state machine graph
wifi_machine.get_graph().draw('wifi/wifi_diagram.png', prog='dot')  
wifi_machine.init()
wifi_machine.sniff() # start Wi-Fi sniffing
```

The model parameters can be either changed in the configuration file or in the python model object during initialization. If the parameters are defined directly in the model object, they take preference over the configuration file. 

<aside class="info"> Check the constructor of Dot11Methods if you need to override the configuration file.</aside>
.

FreeRadius Server is used to perform validation of EAP messages. For now, the configuration of the desired EAP method must be done manually by editing the parameter **default_eap_type** in file **/etc/freeradius/3.0/mods-available/eap**.



<aside class="warning"> If your device doesn't support enterprise network, you can disable the EAP states in Greyhound Wi-Fi model by setting the parameter eap_enable to false.</aside>
.

## Wi-Fi Client model

This model contains basic states executes by Wi-Fi clients when attempting to connect to an Access Point. Similarly to the above Wi-Fi AP model,  this model captures the core design of IEEE 802.11, 802.11i and 802.1X, which enables Greyhound to automatically detect an AP using WPA2-Personal or WPA2-Enterprise and change its transitions accordingly. 

Make sure **EAPModule** library is installed by running **cd wifi/eap_module && make && make install** before running this model.

![wifi_client_mode](source/images/wifi_client_diagram.png)

The model configuration can be changed in the json file **wifi_client_config.json**. The following default configurations are applied:

> JSON - wifi_client_config.json

```json
{
    "WifiSSID": "esp32", 
    "WifiPassword": "mypassword", 
    "WifiUsername": "matheus_garbelini", 
    "WifiChannel": 1, 
    "WifiMAC": "28:c6:3f:a8:af:c5", 
    "WifiSecurity": "CCMP", 
    "WifiInterface": "wlan0mon", 
    "MonitorSerialPort": "/dev/ttyUSB*", 
    "MonitorSerialBaud": "115200", 
    "MonitorDetectionString": "WPA2 ENTERPRISE VERSION:", 
    "EnableFuzzing": true
}
```

| Parameter                  | Default Value           | Description                                                  |
| :------------------------- | :---------------------- | ------------------------------------------------------------ |
| **WifiSSID**               | esp32                   | Wi-Fi network name to connect                                |
| **WifiPassword**           | mypassword              | Wi-Fi password to use                                        |
| **WifiUsername**           | matheus_garbelini       | Username for enterprise network                              |
| **WifiChannel**            | 1                       | Wi-Fi Channel                                                |
| **WifiMAC**                | 28:c6:3f:a8:af:c5       | Model own WI-Fi MAC address                                  |
| **WifiSecurity**           | CCMP                    | Wi-Fi cipher-suite to use during handshake (TKIP or CCMP)    |
| **WifiInterface**          | wlan0mon                | Wi-Fi interface used by Wi-Fi model                          |
| **MonitorSerialPort**      | /dev/ttyUSB*            | Serial monitor port for crash detection. Use `*` as wildcard. |
| **MonitorSerialBaud**      | 115200                  | Serial baudrate.                                             |
| **MonitorDetectionString** | WPA2 ENTERPRISE VERSION | Magic string used for crash detection in the monitored device |
| **EnableFuzzing**          | true                    | Enable or Disable Fuzzing with the pre-defined values in the model file |

> Python - Wi-Fi model initialization

```python
wifi_machine = Dot11Methods(states, transitions)
wifi_machine.get_graph().draw('wifi/wifi_cient_diagram.png', prog='dot')  
wifi_machine.sniff() # start Wi-Fi sniffing
```

The most important parameters are **WifiSSID**, **WifiPassword** and **WifiChannel**. You must configure them according to your access point. Additionally, if enterprise network is being used (EAP), **WifiUsername** must contain a valid username.

The model parameters can be either changed in the configuration file or in the python model object during initialisation. If the parameters are defined directly in the model object, they take preference over the configuration file. 

## Bluetooth Low Energy 4.2 model (WIP)

The Bluetooth Low Energy model handles some features of the 4.2 version such as LE Data Length Extension (DLE). It also support the different pairing modes such as Legacy and Secure Connections. IO Capabilities can then be configured to select the intended pairing method such as Just Works, Numeric Comparison or Passkey entry.

Make sure **SMPServer** library is installed by running **cd wifi/eap_module && pypy setup.py build install** before running this model.

> BLE Stack model

> ![ble_stack](source/images/ble_stack.png)

![img](source/images/ble_central.png)

**Model features:**

* Link Layer encryption (*AES*-*CCM*)
* Basic L2CAP and ATT handling
* Secure Manager Protocol (SMP) which supports Legacy and Secure Connection pairing modes
* GATT Server with a default Generic Attributes Profile service
* GATT Client services discovery

During transmission and reception of data channel PDUs, no HCI is used and the BLE driver automatically handles Acknowledgments (empty PDUs) and retransmissions. This means that Sequence Number (SN) and Next Expected Sequence Number (NESN) flags are automatically overridden by the driver.

> JSON - ble_config.json

```json
{
    "MasterAddress": "5D:36:AC:90:0B:22", 
    "SlaveAddress": "E9:9A:3B:FB:BD:8E", 
    "AccessAdress": "9A328370", 
    "ConnectionInterval": 16, 
    "WindowOffset": 1, 
    "WindowSize": 2, 
    "SlaveLatency": 0, 
    "ChannelMap": "1FFFFFFFFF", 
    "ConnectionTimeout": 100, 
    "MasterFeatureSet": "le_encryption+le_data_len_ext", 
    "DongleSerialPort": "/dev/ttyACM0", 
    "EnableFuzzing": true, 
    "PairingPin": "0000", 
    "MonitorSerialPort": "/dev/ttyACM1", 
    "MonitorSerialBaud": 115200
}
```

The configuration for this model is saved in the **ble_config.json** file. The following parameters can be modified in the model:

| Parameter              | Type    | Description                                                  |
| ---------------------- | ------- | ------------------------------------------------------------ |
| **MasterAddress**      | String  | Initiator/Master MAC address (Greyhound)                     |
| **SlaveAddress**       | String  | Advertiser/Slave MAC address (target)                        |
| **AccessAdress**       | String  | Data channel access address in connection request. It can be any 4 byte value |
| **ConnectionInterval** | Integer | Time between each channel hop during data connection. It represents (value*1.25) milliseconds |
| **WindowOffset**       | Integer | Time offset for the first connection event (anchor point). It represents (value*1.25) milliseconds |
| **WindowSize**         | Integer | Maximum waiting time for the first connection event (anchor point). It represents (value*1.25) milliseconds |
| **SlaveLatency**       | Integer | Number of connection events which the slave can ignore       |
| **ChannelMap**         | String  | Map BLE channels used during connection. Each bit represents a channel |
| **ConnectionTimeout**  | Integer | Supervision timeout. It represents (value*1.25) milliseconds |
| **MasterFeatureSet**   | String  | String representing the announced features of the master (greyhound). Each feature must be separated by `+` |
| **DongleSerialPort**   | String  | Serial port name of the BLE Driver                           |
| **EnableFuzzing**      | Boolean | Enable or Disable Fuzzing with the pre-defined values in the model file |
| **PairingPin**         | String  | The PIN number for Passkey entry method                      |
| **MonitorSerialPort**  | String  | Serial port name of the device being monitored               |
| **MonitorSerialBaud**  | Integer | Serial baud rate of the device being monitored in bits per second |

> Python - BLE model initialization
```python
model = BLECentralMethods(states, transitions,
                          # master_mtu must be 4 bytes less than max RX/TX length
                          master_mtu=247, # 23 is the default, 247 is max
                          slave_address=slave_target,
                          master_address='5d:36:ac:90:0b:22',
                          dongle_serial_port='/dev/ttyACM0',
                          baudrate=115200,
                          # Override enable_fuzzing in the conf. file
                          enable_fuzzing=True)
model.get_graph().draw('bluetooth/ble_central.png', prog='dot')
model.sniff()
```

The model parameters can be either changed in the configuration file or in the python model object during initialization. If the parameters are defined directly in the model object, they take preference over the configuration file.

<aside class="info"> Check the constructor of BLECentralMethods if you need to override the configuration file.</aside>
.

# :arrow_forward: Running Greyhound

> Shell - Greyhound help
```bash
chmod +x greyhound.py # Mark greyhound.py as executable
./greyhound.py --help
```
> ![greyhound_help](source/images/greyhound_models.png)

Give greyhound.py executable permission before using it. Alternatively you can start it by using `python greyhound.py`. You can retrieve the list of available models for greyhound by using the command `./greyhound.py --help` . Note that **wifi_client** model is not available at the moment.

| **Command**                    | **Description**                                       |
| ------------------------------ | ----------------------------------------------------- |
| **./greyhound.py wifi_ap**     | Start an wi-fi access point model. Fuzz Wi-Fi clients |
| **./greyhound.py wifi_client** | Start an wi-fi clientmodel. Fuzz Wi-Fi access point   |
| **./greyhound.py ble_central** | Start an BLE central model. Fuzz BLE Peripherals      |

greyhound.py is a daemon that runs the model in background. If the model process ends due to some bug or configuration update, the daemon will restart it automatically.

## Wi-Fi fuzzing

> Shell - Start Wi-Fi model

```bash
sudo ./greyhound.py wifi_ap
```

> ![wifi_output](source/images/wifi_output.png)

<aside>Ensure that the wifi model has the parameter FuzzingInterface matching your Wi-Fi interface before continuing.</aside>
.

By executing **sudo ./greyhound.py wifi_ap**, the terminal will show many logs, including network name, default gateway IP and the selected. Additionally, probe responses will be transmitted if nearby Wi-Fi devices are scanning the network on the same channel which the Wi-Fi model is configured. You can also confirm that Greyhound is working by checking your smartphone Wi-Fi list. 

Note that a new network interface (iface) with the name of the model SSID will be created on your system. You can run **ifconfig** to list this interface which should have its IP the same as the model gateway IP. You can use this interface to connect to the tested device via SSH or access its web server via a browser if such device has one. When the new device connects to the Wi-Fi AP model, and it has DCHP enabled, it should receive the IP 192.168.42.2 by default. Running **ping 192.168.42.2** should return responses from the tested device. Be aware that It's necessary to remove the timeout condition of the last state **ANALYZE_DATA** so the tested device is kept connected to the model.

Check [Running optimization](#running-optimization) to start the automatic fuzzing process.

<aside>If you are testing a Wi-Fi device which supports serial connection for crash detection, remeber to insert such device on your system before the model starts.</aside>
.

## BLE fuzzing

> Shell - Start BLE model

```bash
sudo ./greyhound.py ble_central
```

> ![ble_output](source/images/ble_output.png)

Before starting the BLE model, make sure that the Slave Address is configured in **ble_central.py** or the configuration file **ble_config.json**. If you don't know the advertisement address, you can use Android/IOS apps that can scan BLE devices. Be aware that it's recommended to realize testing with BLE devices that are using public advertisement addresses. As random advertisement addresses are randomly update after a certain time, the model will eventually not be able to reconnect to the device. Future support is planned to dynamically search for device's name in the scan response for such case.

By executing **sudo ./greyhound.py ble_central**, the terminal will show many logs, including packets transmitted and sent, current state, transitions and timeout. The model will attempt to scan and connect to the BLE device being tested according to its advertising address.

**(WIP)** Check [Running optimization](#running-optimization) to start the automatic fuzzing process.

<aside>If you are testing a BLE device which supports serial connection for crash detection, remeber to insert such device on your system before the model starts.</aside>
.

## Running optimization

> Shell - Run fuzzing optimization

```bash
python bench.py
```

> Python - Available cost functions

```python
def fitness(self, x):
        # Send new fuzzing data
        x_int = [int(value) for value in x]
        print('Input (' + str(len(x)) + '): ' + str(x_int))
        self.interface[0].SetConfig(x_int)
        self.interface[0].iteration_lock.acquire()

        # Choose one of the following cost functions

        # wifi_fitness = - self.interface[0].transitions
        # wifi_fitness = self.interface[0].issue_period
        # wifi_fitness = self.interface[0].iteration_time
        wifi_fitness = - self.interface[0].issue_count
```

In order to enable Greyhound to test a model in a feed-back driven manner. It's necessary to run **bench.py**. This script will connect via [SocketIO](https://socket.io/) to the model Web API and leverage updates to the fuzzing probabilities based on the cost functions returned by the model. Several iterations will be attempted with the tested device. After several generations, a file **log.txt** is generated, containing information of the fitness, population and input values of each generation in addition with the probabilities array of the best overall individual. 

By default, the process stops after 200 generations or 1000 iterations, and the default cost function selected to be minimized is anomaly count (issue_count), but you can change this by uncommenting one of the lines from the **fitness** method. After the optimization ends, a graph of the fitness over all iterations is generated and shown to the user and it's data is saved in **logs/graph.csv**.

The figure below shows a typical output log of the bench.py.

![bench](source/images/bench_output.png)

It's important to mention that Greyhound was design to support optimisation feature isolated from it's main functionalities. As such, you can skip the installation of pagmo2 (used in **bench.py**) and install in a different machine instead. This is possible because **bench.py** communication with the model occurs via Socket.IO, thus, any machine which is in the same network as the device running Greyhound can perform the optimization instead. It's even possible to perform the optimization in other frameworks such as the ones in Matlab by simply using [Greyhound Web API](#----web-api).

## Crash detection

If the device being tested supports USB Serial/COM output to indicate its operational status, GreyHound can detect a magic string received via serial to indicate such crash. Such parameter is `crash_magic_word` in the initialisation object of your chosen model.

Additionally, for other devices, a SocketIO endpoint is exposed as `SignalCrash` and can be used to notify GreyHound of a crash. More details can be seen in [Web API](#----web-api).



# :page_facing_up: **Anomaly Logs**

During the fuzzing process, all captures, logs and summary tables are saved in the folder `logs`. You can check if any anomalies were found by opening the filed under `logs/csv`. A Wireshark log is generated after certain event and saved with specific prefixes:

* **session_$n** - Saved after each iteration with the tested device
* **crash_$n** - Saved after every crash occurred during any fuzzing session
* **anomaly_$n** -  Saved after every anomaly occurred during any fuzzing session

The **logs/pcap** folder saves shortened captures of only the relevant packets during the fuzzer process. However, this should not be always used to decide if a anomaly/crash occurred as the tested device may not have crashed or performed an anomaly immediately after receiving a fuzzed packet.

The **logs/csv** is used to assist with the anomaly detection and could be used as a sort of fingerprint to create a known vulnerabilities database **(WIP)**. The typical CSV summary contains the following columns:

* **TIME** - Timestamp of the anomaly detected. Example: 09/09/2019  04:36:43

* **STATE** - Name of the state of which the anomaly was triggered. Example: WAIT_AUTH_REQUEST

* **RECEIVED_PKT** - Packet summary of the last received packet when the anomaly was triggered. Example:  `Dot11 / LLC / SNAP / EAPOL / EAP Response PEAP`

* **REASON** - Reason summary for the anomaly detection. Example: 

  `CRASH detected in state WAIT_AUTH_REQUEST` or 

  `ANOMALY detected in state WAIT_AUTH_REQUEST`

* **FUZZED_PKT** - Packet summary of the last fuzzed packet sent before the anomaly was triggered. Example: 

  `802.11 Management 8 / Dot11Beacon / SSID / Dot11EltRates / Dot11Elt / Dot11EltRSN`

* **FUZZED_FIELDS_NAME** - Name of the mutated fields of the last packet fuzzed. Example: 

  `['code', 'L']`

* **FUZZED_FIELDS_VALUE** - Fields value of the mutated fields of the last packet fuzzed. Example:

  `[[204, 52931]]`

* **DUPLICATED_PKT** - Name of the last function which was repeated before triggering the anomaly. Example: `send_wpa_handshake_3`

**The CSV summary alone only gives an overview of the possible anomaly. To confirm if a certain anomaly indeed is a security vulnerability, the Wireshark captures need to be verified manually.**

# :computer: User Interface (GUI)

The graphical user interface of Greyhound was initially developed to provide simple monitoring view of the fuzzing process. The responsive interface is built using web technologies under [Quasar Framework](https://v0-17.quasar-framework.org/quasar-play/apple/index.html#/showcase).

> GUI - Wireless models option (Wi-Fi and BLE)

> ![gui_cong](source/images/gui_conf.png)

**![gui](source/images/demo.png)**

To support a generic visualization of any model state machine, [GraphViz](https://www.graphviz.org/) state graph is automatically updated by Greyhound state machine and sent to the GUI by Greyhound Webserver.  The GUI also provides easy configuration of the model by clicking on the **Model Options** button. 

If the GUI cannot be opened in the same platform Greyhound is being executed, it's possible to distribute the GUI to other device by coping the folder **gui/dist/spa**. As the GUI is simply a web page, it behaves as a Desktop application or Web application depending on your requirements. By default the GUI attempts to connect to Greyhound via **127.0.0.1:3000**, however the IP address and port for the host running Greyhound can be changed by clicking on the engine icon from the left panel menu.

## Desktop App

To open the GUI natively on your system, [NWJS](https://nwjs.io/) project is used to run the APP as a desktop application. Gryhound already wraps such project in **gui** folder. The executable for Linux X86-64 distributions is in **gui/GreyhoundGUI**.

If you which to modify the GUI, you need to install NodeJS dependencies (**npm install**) and execute **npm run build** for live-reloading development or **npm run build** to simply build the GUI to **dist** folder.

## **Web App**

The GUI can be served as an web app by hosting the files within the folder **gui/dist/spa**, however opening **gui/dist/spa/index.html** in the browser also works if using google chrome. Alternatively, you can access the GUI by clicking on **[Greyhound Web GUI](app/index.html)** in the left panel menu of this documentation.

![panel_gui](source/images/panel_gui.png)

# :notebook_with_decorative_cover: ​Web API
Greyhound webserver (**greyhound/webserver.py**) initializes a SocketIO server with the following endpoints:

| Endpoint            | Input | Output | Description                                                  |
| ------------------- | ----- | ------ | ------------------------------------------------------------ |
| **Reset**           | None  | None   | Reset/Shutdown model                                         |
| **SignalCrash**     | None  | None   | Indicate that a monitored device has crashed                 |
| **GraphDot**        | None  | String | Returns graph of model in GraphDot format                    |
| **GetFuzzerConfig** | None  | List   | Returns probabilities array                                  |
| **SetFuzzerConfig** | List  | None   | Changes probabilities array                                  |
| **GetFitness**      | None  | JSON   | Return model fitness                                         |
| **GetModelConfig**  | None  | JSON   | Return model configuration file                              |
| **SetModelConfig**  | JSON  | None   | Changes model configuration according to its configuration file |

Several SocketIO client libraries can be used to interact with the above endpoints. This libraries are available for [Python](https://python-socketio.readthedocs.io/en/latest/client.html), [C](https://github.com/IBM/socket-io), [C++](https://github.com/socketio/socket.io-client-cpp), [C#](https://github.com/IBM/socket-io), [JavaScript](https://socket.io/), [Java](https://github.com/socketio/socket.io-client-java), etc.

<aside>SetFuzzerConfig must be called with the states fuzzing configuration before this API can be fully used. Make sure to call SetFuzzerConfig before instanciating any wireless model class.</aside>
.

# :hammer: Development tools

Greyhound allows fast development by already having configuration in some of its folders by the following tools:

* **PyCharm Community Edition** - Used for Python code. Simply open Greyhound project root folder and the IDE should be already configured with the correct imports, interpreter and scripts.

![ide_pycharm](source/images/ide_pycharm.png)

* **Visual Studio Code** - Used for C/C++ code. You can open **drivers/RT2800** or **drivers/NRF52_Dongle** with this IDE to edit and build the drivers. Autocompletion, include folders and build tasks are already configured so you start to modify and test the codes immediately.

![ide_pycharm](source/images/ide_visual_code.png)

#  :memo: TODO

Here's the summary of proposed improvements of Greyhound and its models:

* Add support for Wi-Fi Supplicant model
* Add support for Bluetooth Classic model
* Create databased of known vulnerabilities based on CSV and capture logs
* Simplify the installation process to support common Linux platforms
* Allow easy configuration of the optimization process with a simplified coverage solution
* Add progress reporting of the optimization process
* Add BLE link slave (advertiser) in the NRF52_Dongle driver



# :email: Contact

**If you face any problem during installation or when running the tool, please contact matheus_garbelini@mymail.sutd.edu.sg**

