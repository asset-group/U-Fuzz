# ⏩ Quick Start Guide

**WDissector** fuzzer is the new iteration of the older GreyHound fuzzer. It now offers fuzzing integration with wireshark to extend supported protocols and performance. This quick start guides on how to download and run the binary releases and alternativelly how to compile it from sources. It includes by default a custom version of Wireshark with some new dissectors.

::: tip Repository Link

WDissector repository is maintained on GitLab and requires authorisation. 
You can access the repository here: [https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/tree/wdissector](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/tree/wdissector)

Note that WDissector is on `wdissector` branch.

:::

## Compatible OS

::: tip OS Compatibility

The release has been tested and confirmed to work on both **Ubuntu 18.04** and **Ubuntu 20.04**.

:::

## 1) Download Binary Release

The latest release can be downloaded either via Web Browser or via command line.

* Via **Web Browser** (Requires GitLab login): [https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/raw/release/wdissector.tar.zst?job=release](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/raw/release/wdissector.tar.zst?job=release)

* Or Via **bash command line**:

  ```bash
  wget --header="PRIVATE-TOKEN: -5AEDnLgc6u-SV_zPAY5" \
  --content-disposition "https://gitlab.com/api/v4/projects/\
  asset-sutd%2Fsoftware%2Fwireless-deep-fuzzer/jobs/artifacts/\
  wdissector/raw/release/wdissector.tar.zst?job=release"
  ```

Next, extract the downloaded release file (wdissector.tar.zst) as follows:

```bash
# Install zstandard
sudo apt install zstd
# Extract the wdissector compressed file
tar -I zstd -xf wdissector.tar.zst
cd wdissector
```



## 1.1) Compile from source (optional)

Several requirements needs to be installed before compiling the project. An automated script for Ubuntu 18.04/20.04 is provided on `requirements.sh`. To compile from source, simply run the following commands:

```bash{3,5}
git clone https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer -b wdissector --depth=1
cd wdissector
./requirements.sh dev # Install all requirements to compile wdissector from source
./requirements.sh doc  # Install nodejs requirements to generate documentation
./build.sh all # Compile all binaries. It may take around 15min. Go get a coffe!
./build.sh doc # Generate documentation
```





## 2) Flash BT firmware

This step is required only for BT fuzzing. The custom BT firmware needs to be uploaded to an ESP32 board with FT232H or FT2232H high speed serial to usb chip. The recommended board to use is [ESP-WROVER-KIT-VE](https://www.espressif.com/en/products/hardware/esp-wrover-kit/overview)

Download the ESP32 firmware package as follows:

* Via **Web Browser** (Requires GitLab login): [https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/raw/release/esp32driver.zip?job=release](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/raw/release/esp32driver.zip?job=release)

* Or Via **bash command line**:

  ```bash
  wget --header="PRIVATE-TOKEN: -5AEDnLgc6u-SV_zPAY5" \
  --content-disposition "https://gitlab.com/api/v4/projects/\
  asset-sutd%2Fsoftware%2Fwireless-deep-fuzzer/jobs/artifacts/\
  wdissector/raw/release/esp32driver.zip?job=release"
  ```

You can upload the firmware to ESP32 on either Windows or Linux. For windows you need to manually install python3 before executing the installation script.

<code-group>
<code-block title="Windows">

```shell{2}
cd release
python firmware.py COM6
```
</code-block>

<code-block title="Linux (Ubuntu)" active>

```bash{4}
sudo apt install unzip python3-dev
unzip esp32bt.zip # Extract esp32bt.zip if you haven't done so
cd release
python3 firmware.py /dev/ttyUSB0 # Please change your serial port to match your device.
```
</code-block>
</code-group>

::: warning

You may need to press and hold the `boot` button during the firmware upload process if you get failures.

:::

## 3) Running the fuzzer

Before running the fuzzer, you need to install few system libraries that the binary release requires. You can do so by executing `requirements.sh` on wdissector root folder. The steps are shown below:

```bash{4}
# Install few requirements
./requirements.sh
# Run one of the following binaries
sudo bin/bt_fuzzer # BT Fuzzer
sudo bin/bt_fuzzer --no-gui # BT Fuzzer without GUI interface
sudo bin/lte_fuzzer # LTE Fuzzer
bin/wireshark # Standalone Wireshark GUI with custom dissectors (Use this to see the logs)
```



## 4) Usage

* 🔀 **Fuzzers -** See more options and usage on [Fuzzers](/fuzzers) section.
* ☁️ **Web API -** Integrate fuzzing to your workflow via the integrated SocketIO server. 
  Examples on how to control the fuzzer or receive events remotely is presented on section [Web API](/api).



