# U-Fuzz: Stateful Fuzzing of IoT Protocols on COTS Devices
U-Fuzz is a framework to systematically discover and replicate security vulnerabilities on arbitrary wired and wireless IoT protocol (e.g., CoAP, Zigbee, 5G NR) implementations. U-Fuzz offers possibility to automatically construct the protocol state machine with only a few packet traces of normal (i.e., benign) communication.

<p align="center">
  <img src="figs/Uni_overal_Design.png" alt="U-Fuzz Overview and Design">
</p>

------

**Table of Contents**

1. [📋 Software Environment](#1-📋-software-environment)

2. [⏩ Initial Compilation](#2-⏩-initial-compilation)

3. [🔀 Running Multi-protocol Statemapper](#3-🔀-running-multi-protocol-statemapper)
    * [Manual Mode Running details](#31-manual-mode-running-details)
    * [Model representation figure](#model-representation-figure)

4. [🧑‍💻 Running the fuzzer](#4-🧑‍💻-running-the-fuzzer)
    * [Zigbee](#41-zigbee)
    * [CoAP](#42-coap)
    * [5G](#43-5g)
        * [5G Container](#431-5g-container)
        * [Fuzzing 5G with 5G Simulator](#432-fuzzing-5g-with-5g-simulator)
        * [Fuzzing 5G with real COTS](#433-fuzzing-5g-with-real-cots)
5. [📄 Exploits](#5-📄-exploits)
  * [Summary of CVEs](#51-summary-of-cves)
  * [Available Exploits](#52-available-exploits)
    * [V14 replication](#521-v14-replication)
6. [📝 Citing U-Fuzz](#6-📝-citing-u-fuzz)



------

# 1. 📋 Software Environment
**OS:** Ubuntu 18.04
**Wireshark Version:** V4.1 (patched)
**Bindings:** Python3, Golang

# 2. ⏩ Initial Compilation 
Several requirements needs to be installed before compiling the project. An automated script for Ubuntu 18.04 is provided on `requirements.sh`. To compile from source, simply run the following commands:
```
$ Download the content from this github link:
https://anonymous.4open.science/r/cots-iot-fuzzer/

$ cd wdissector

$ ./requirements.sh dev # Install all requirements to compile wdissector from source

$./requirements.sh doc  # Install nodejs requirements to generate documentation

$./build.sh all # Compile all binaries. It may take around 15min. Go get a coffe!
```

# 3. 🔀 Running Multi-protocol Statemapper
Before running the fuzzer, the multi-protocol-Statemapper need to be run to generate both the configuration file and the state model.

The multi-protocol-statemapper needs two inputs,

1: Capture_trace_for target_protocol.pcapng(can input via terminal)
2: configuration template to append the mapping rules needs to specify the file name in "multi_protocol_state_mapper.py :line: 423"
After compile from the project with the correct software environment, please run the following command with the two inputs mentioned previously.
```
$ cd .../cots-iot-fuzzer/multi_protocol_statemapper/wdissector

$ python3 multi_protocol_state_mapper.py
```

## 3.1 Manual Mode Running details

Step1: input the final capture trace
<p align="center">
  <img src="figs/Step1.png" alt="figStep1">
</p>
```
user can choose to combine multiple capture file by themselves
or use U-Fuzz combiner by input y.
```

Step2: After the capture analysis all the potential state and packet list 
will be print out, user can proceed to form new state by entering y then the 
potential packet list (can copy and paste from above).

<p align="center">
  <img src="figs/step2.png" alt="figStep2">
</p>

<p align="center">
  <img src="figs/Step2_follow.png" alt="fig-2follow">
</p>

After the initial packet list was input, if the user wants to continuously input packets,
they can input 'y' for the follow question, or can just input 'n' to proceed.

<p align="center">
  <img src="figs/step3.png" alt="figStep3">
</p>

Step3: After the potential packet list was input, the capture processor will out put all the 
common layers which shared by all packets the user just input, then analysis from the most 
relevant layer.

<p align="center">
  <img src="figs/step4.png" alt="figStep4">
</p>

In the mean time, the user needs to input a name for that state, can copy and paste from above also.

<p align="center">
  <img src="figs/Step4_follow.png" alt="fig-4follow">
</p>

Step4: one by one analysis will be performed

<p align="center">
  <img src="figs/step5.png" alt="figStep5">
</p>

then followed by 2 by 2 if filer was not found by 1 by 1.

<p align="center">
  <img src="figs/twobytwo.png" alt="fig2by2">
</p>

Step5: Once the filter is found, user can decide to continuously create new state by input 
'y' or stop by input 'n'.

<p align="center">
  <img src="figs/step5.png" alt="figStep7">
</p>

Step6: Once 'n' was input for the previous step, three input will be asked for the statemachine 
generation

1. previous inputed Capture file 
2. Tamplete config file
3. The outfile name, (.json is for the statemachine generatoin, .png is for the statemachine image generation)


<p align="center">
  <img src="figs/Step8.png" alt="figStep8">
</p>


## Model representation figure

<p align="center">
  <img src="figs/model-fig.png" alt="figmodel">
</p>



# 4. 🧑‍💻 Running the fuzzer
## 4.1 Zigbee

**Step1:**
*build the project (zigbee_real_time_fuzzer)*

```
Edit the CMakeLists.txt
$ Uncomments line:802, 810-814 
  (`set(ZIGBEE_SRC src/zigbee_real_time_fuzzer.cpp libs/shared_memory.c)`)
  (`add_executable(zigbee_real_time_fuzzer ${ZIGBEE_SRC} libs/profiling.c)`)
  (`target_link_libraries(zigbee_real_time_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS} viface)`)
  (`target_compile_options(zigbee_real_time_fuzzer PRIVATE -w -O0)`)
  (`target_compile_definitions(zigbee_real_time_fuzzer PRIVATE -DFUZZ_ZNP)`)
$ Comments line: 804, 824-828 which were configured for CoAP fuzzing
  (`set(COAP_SRC src/coap_realtime_fuzzer.cpp libs/shared_memory.c)`)
  (`add_executable(coap_realtime_fuzzer ${COAP_SRC} libs/profiling.c)`)
  (`target_link_libraries(coap_realtime_fuzzer PRIVATE ${MINIMAL_FUZZER_LIBS} viface)`)
  (`target_compile_options(coap_realtime_fuzzer PRIVATE -w -O0)`)
  (`target_compile_definitions(coap_realtime_fuzzer PRIVATE -DFUZZ_WIFI_AP)`)

$ ./build.sh all

```
**Step2:**

*install Zigbee2Mqtt *from link: https://www.zigbee2mqtt.io/
the sample configuration file is located at 
cots-iot-fuzzer/zigbee_dongle_connection/coordinator/data

**Step3:**

*prepare the hardware * for fuzzing Zigbee including coordinator dongle (e.g., CC2531 ZNP-Prod)
and zigbee smart devices 

![figdongle](figs/zigbeedongle.jpg "title-8")

**Step 4:**
*Run the fuzzer *

```
open a new terminal then run 
$ mosquitto

run the fuzzer at directory
$ cd cots-iot-fuzzer/
$ sudo bin/zigbee_real_time_fuzzer --EnableMutation=true

$ cd zigbee_dongle_connection/coordinator
$ docker-compose up
```

## 4.2 CoAP

**Step1:**
*build the project (coap_realtime_fuzzer)*
```
$ ./build.sh all

```
**Step2:**
Set up target CoAP sever implementation (e.g., Libcoap)

**Step3:**
*Run the Fuzzer*
```
$ Run CoAP Server

$ cd cots-iot-fuzzer/
$ sudo bin/coap_realtime_fuzzer --EnableMutation=true
$ cd cots-iot-fuzzer/coap_client_server
$ sudo ip netns exec veth5 node client_complete.js
```
## 4.3 5G

### 4.3.1 5G Container:
Download the 5G container from the docker hub

Credential: 

```
docker login -u a80568681433

Access token:
dckr_pat_A7VRSeNGp_tJPhIAuk4Iksk0pxM
```

### 4.3.2 Fuzzing 5G with 5G Simulator:

$ cd */wireless-deep-fuzzer/5gcontainer

$ chmod +x container.sh

$ ./container.sh run release-5g

Use the following command to just run 5G simulator
$ sudo bin/lte_fuzzer --EnableSimulator=true


### 4.3.3 Fuzzing 5G with real COTS: 
Hardware Preparation:

  [OnePlus Nord CE2](https://www.oneplus.com/sg/nord-ce-2-5g/specs) or other 5G COTS UE.

  [USRP B210](https://www.ettus.com/all-products/usrp-b200-enclosure/)

Command:

```
$ chmod +x container.sh

$ ./container.sh run release-5g

$ sudo bin/lte_fuzzer  --EnableSimulator=false

```

# 5. 📄 Exploits
## 5.1.  Summary of CVEs:
As of today, U-Fuzz has discovered 11 new security flaws which have been assigned 11 CVE IDs. The correspondence between the exploit name and U-Fuzz discovered vulnerability is shown in the Table below:

| Protocol Under Test | U-Fuzz Vulnerability Name                             | Affected Hardware/Software Implementation | CVE            |
| --------------------|-------------------------------------------------------|-------------------------------------------|----------------|
| 5G                  | V1 - Invalid CellGroupConfig                          | OnePlus Nord CE 2                         | CVE-2024-20004           |
| 5G                  | V2 - Invalid CellGroupId                              | OnePlus Nord CE 2                         | CVE-2024-20003           |
| 5G                  | V3 - Invalid RLC Sequence                             | OnePlus Nord CE 2                         | CVE-2023-20702 (existed) |
| 5G                  | V4 - Invalid Uplink Config Element                    | OnePlus Nord CE 2                         | CVE-2023-32843 (existed) |
| 5G                  | V5 - Null Uplink Config Element                       | OnePlus Nord CE 2                         | CVE-2023-32845 (existed) |
| Zigbee              | V6 - Invalid Transaction and Cluster ID               | Texas Instrument CC2531 USB Dongle Z-stack version: Z-Stack_Home_1.2 SONOFF Zigbee 3.0 USB Dongle-P Z-stack version: Z-Stack_3.0.x | CVE-2023-41388           |
| Zigbee              | V7 - Invalid Transaction and Cluster ID               | Zigbee2Mqtt Version:3.8                   | CVE-2023-41003           |
| Zigbee              | V8 - Malformed AF_Data_Request                        | Zigbee2Mqtt Version:3.8                   | CVE-2023-42386           |
| Zigbee              | V9 - Out of Sync State Information                    | Zigbee2Mqtt Version:3.8                   | CVE-2023-41004           |
| Zigbee              | A1 - Skip Link Status                                 | Tuya Smart Plug                           | Not applicable           |
| Zigbee              | A2 - Skip Link Status                                 | Philips Hue Smart Light Bulb              | Not applicable           |
| CoAP                | V10 - NullPointerException                            | Jcoap                                     | CVE-2023-34918           |
| CoAP                | V11 - Illegal_Argument_Exception_Invalid_Token_Length | Jcoap                                     | CVE-2023-34920           |
| CoAP                | V12 - Slice_Bounds_out_of_Range                       | Canopus                                   | CVE-2023-34919           |
| CoAP                | V13 - Bad Get Request                                 | Canopus                                   | CVE-2023-34921           |
| CoAP                | V14 - Invalid Size1 Size2 Options                     | libcoap                                   | CVE-2023-33605           |
| CoAP                | V15 - Bad POST Request                                | CoAPthon                                  | CVE-2018-12680 (existed) |
| CoAP                | V16 - Invalid Unicode Decoding                        | CoAPthon                                  | CVE-2018-12680 (existed) |

## 5.2. Available Exploits
| U-Fuzz Vulnerability Name                             | Exploit                |
| ----------------------------------------------------- | ---------------------- |
| V1 - Invalid CellGroupConfig                          | [mac_sch_mtk_rrc_setup_crash_2](/modules/exploits/5gnr_gnb/mac_sch_mtk_rrc_setup_crash_2.cpp)|
| V2 - Invalid CellGroupId                              | [mac_sch_mtk_rrc_setup_crash_1](/modules/exploits/5gnr_gnb/mac_sch_mtk_rrc_setup_crash_1.cpp)|
| V3 - Invalid RLC Sequence                             | [mac_sch_mtk_rlc_crash](/modules/exploits/5gnr_gnb/mac_sch_mtk_rlc_crash.cpp)|
| V4 - Invalid Uplink Config Element                    | [mac_sch_mtk_rrc_setup_crash_3](/modules/exploits/5gnr_gnb/mac_sch_mtk_rrc_setup_crash_3.cpp)|
| V5 - Null Uplink Config Element                       | [mac_sch_mtk_rrc_setup_crash_4](/modules/exploits/5gnr_gnb/mac_sch_mtk_rrc_setup_crash_4.cpp)|
| V10 - NullPointerException                            | [replicate_crash_jcoap_1](/CoAP_Crash/replicate_crash_jcoap_1.py)|
| V11 - Illegal_Argument_Exception_Invalid_Token_Length | Can replace replicate_crash_jcoap_1 to any PUT request | 
| V12 - Slice_Bounds_out_of_Range                       | [replicate_crash_canpous](/CoAP_Crash/replicate_crash_canpous.py)|
| V13 - Bad Get Request                                 | [replicate_crash_canpous](/CoAP_Crash/replicate_crash_canpous.py)|
| V15 - Bad POST Request                                | [replicate_crash_CoAPthon](/CoAP_Crash/replicate_crash_CoAPthon.py)|
| V16 - Invalid Unicode Decoding                        | [replicate_crash_CoAPthon](/CoAP_Crash/replicate_crash_CoAPthon.py)|

### 5.2.1 V14 replication
Our group use Esp32 board to fuzz libcoap and replicate the V14 we found on libcoap, the detailed replication toturial 
could be found at [Libcoap_crash_replication_toturial](/CoAP_Crash/Libcoap_crash_replication_toturial.html). The replication 
script can be found at [replicate_crash_libcoap](/CoAP_Crash/replicate_crash_libcoap.py).

# 6. 📝 Citing U-Fuzz

```
@article{
  author={Shang, Zewen and Garbelini, Matheus E and Chattopadhyay, Sudipta},
  booktitle={2024 IEEE Conference on Software Testing, Verification and Validation (ICST)}, 
  title={U-Fuzz: Stateful Fuzzing of IoT Protocols on COTS Devices}, 
  year={2024},
}
```