# :gear: Interface Drivers and Hardware Support

This sections describes in more detail the driver requirements (software or hardware) and the inner working of each driver, including their available APIs.

WDissector use drivers to communicate with the fuzzing interface. This interface can require a specific hardware or software or a combination of them. 

Currently there are drivers for Bluetooth classic and LTE (OpenAirInterface). The current Drivers are listed on the table below.

| Protocol Target                 | Interface Driver Name | Interface Communication                                 | Hardware Support                                             |
| ------------------------------- | --------------------- | ------------------------------------------------------- | ------------------------------------------------------------ |
| Bluetooth Classic<br />BLE Host | ESP32BTDriver         | Serial Port over High-Speed USB                         | 1. [ESP-WROVER-KIT-VE](https://www.mouser.sg/ProductDetail/Espressif-Systems/ESP-WROVER-KIT-VE) |
| LTE / 4G (eNB)                  | OAICommunication      | Shared Memory + Mutex                                   | 1. [USRP B210](https://www.ettus.com/all-products/ub210-kit/) |
| Wi-Fi 802.11                    | WifiRT8812AUDriver    | [Kernel Netlink](https://en.wikipedia.org/wiki/Netlink) | 1. RT8812AU Compatible Wi-Fi Dongle.<br />> [Alpha AWUS036AC](https://www.amazon.sg/Long-Range-Dual-Band-Wireless-External-Antennas/dp/B00MX57AO4/ref=sr_1_4) |



## ESP32BTDriver

### Hardware

<img src="./figures/drivers/esp-wrover-kit.jpg" alt="esp-wrover-kit" style="zoom:50%;" />

 The recommended hardware is [ESP-WROVER-KIT-VE](https://www.espressif.com/en/products/hardware/esp-wrover-kit/overview). You can buy this board from several electronics suppliers and online stores such as:

* [Mouser](https://www.mouser.sg/ProductDetail/Espressif-Systems/ESP-WROVER-KIT-VE?qs=KUoIvG%2F9Ilbci6DcltJYaA%3D%3D)
* [Digikey](https://www.digikey.sg/product-detail/en/espressif-systems/ESP-WROVER-KIT-VE/1965-ESP-WROVER-KIT-VE-ND/13584249)
* [LCSC](https://lcsc.com/product-detail/Development-Boards-Kits_Espressif-Systems-ESP-WROVER-KIT-VB_C571186.html)
* [Adafruit](https://www.adafruit.com/product/3384)
* [AliExpress](https://www.aliexpress.com/item/4001184374360.html)



### Installing Driver

This driver relies on a custom firmware which must be flashed (installed) on a ESP32 Device. This procedure requires running a script which will automatically download the required tools to flash the custom firmware to ESP32 (via platformio framework). You can run the following on either `Windows 10` or `Ubuntu 18.04 / 20.04`.



Download the ESP32 firmware package as follows:

* Via **Web Browser** (Requires GitLab login): [https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/raw/release/esp32driver.zip?job=release](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/raw/release/esp32driver.zip?job=release)

* Or Via **bash command line**:

  ```bash
  wget --header="PRIVATE-TOKEN: <gitlab token>" \
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



## OAICommunication

### Hardware

<img src="./figures/drivers/usrpb210.png" alt="usrpb210" style="zoom:50%;" />

The LTE fuzzer, which uses OpenAirInterface for the LTE stack, requires use of an SDR. We recommend the use of [USRP B210](https://www.ettus.com/all-products/ub210-kit/) which can be acquired directly from [ETTUS](https://www.ettus.com/all-products/ub210-kit/). However, OAI also supports [other SDRs](https://gitlab.eurecom.fr/oai/openairinterface5g/-/wikis/OpenAirSystemRequirements#supported-rf) which we didn't test yet.

### Installing Driver

The custom OAI/Open5GS software which LTE fuzzer requires can be downloaded and built as follows:

```bash
# Go to wdissector root folder
cd 3rd-party
git clone https://gitlab.com/asset-sutd/software/oai_lte_network --depth=1
cd oai_lte_network/openair-ran
# Requirements will be installed to the system and build process will start
./build_all.sh
# Build Open5GS
cd ../../open5gs-core
./build.sh
```