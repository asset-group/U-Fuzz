# Stateful Fuzzing of IoT Protocols on COTS Devices
COTS_IoT_Fuzzer is a framework to systematically discover and replicate security vulnerabilities on arbitrary wired and wireless IoT protocol (e.g., CoAP, Zigbee, 5G NR) implementations. COTS_IoT_Fuzzer offers possibility to automatically construct the fuzzing statemachine with only a few packet traces of normal (i.e.,begin) communication. 

![U-Fuzz Overview and Design](figs/Uni_overal_Design.png)


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
        * [Approach 1](#431-approach-1)
        * [Approach 2](#432-approach-2)
    * [📄 Summary of CVEs](#summary-of-cves)

5. [📝 Citing U-Fuzz](#5-📝-citing-u-fuzz)



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
![figStep1](figs/Step1.png "title-1")
```
user can choose to combine multiple capture file by themselves
or use U-Fuzz combiner by input y.
```

Step2: After the capture analysis all the potential state and packet list 
will be print out, user can proceed to form new state by entering y then the 
potential packet list (can copy and paste from above).

![figStep2](figs/step2.png "title-2")

![figStep2follow](figs/Step2_follow.png "title-2follow")

After the initial packet list was input, if the user wants to continuously input packets,
they can input 'y' for the follow question, or can just input 'n' to proceed.

![figStep3](figs/step3.png "title-3")

Step3: After the potential packet list was input, the capture processor will out put all the 
common layers which shared by all packets the user just input, then analysis from the most 
relevant layer.

![figStep4](figs/step4.png "title-4")

In the mean time, the user needs to input a name for that state, can copy and paste from above also.

![figStep4follow](figs/Step4_follow.png "title-4follow")

Step4: one by one analysis will be performed

![figStep5](figs/step5.png "title-5")

then followed by 2 by 2 if filer was not found by 1 by 1.

![fig2by2](figs/twobytwo.png "title-6")

Step5: Once the filter is found, user can decide to continuously create new state by input 
'y' or stop by input 'n'.

![figStep7](figs/step5.png "title-7")

Step6: Once 'n' was input for the previous step, three input will be asked for the statemachine 
generation

1. previous inputed Capture file 
2. Tamplete config file
3. The outfile name, (.json is for the statemachine generatoin, .png is for the statemachine image generation)

![figStep8](figs/Step8.png "title-8")



## Model representation figure
![figmodel](figs/model-fig.png "title-7")




# 4. 🧑‍💻 Running the fuzzer
## 4.1 Zigbee

**Step1:**
*build the project (zigbee_real_time_fuzzer)*

```
Edit the CMakeLists.txt
$ Uncomments line:802, 810-814
$ Comments line: 804, 824-828

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

### 4.3.1 Approach 1:

$ cd */wireless-deep-fuzzer/5gcontainer

$ chmod +x container.sh

$ ./container.sh run release-5g

Use the following command to just run 5G simulator
$ sudo bin/lte_fuzzer --EnableSimulator=true

if you want to run 5G fuzzing with real UE, please contact me before running the experiment then i can prepare the fuzzing setup for testing.

### 4.3.2 Approach 2: 

Download the 5G container from the docker hub

Credential: 

```
docker login -u a80568681433

Access token:
dckr_pat_A7VRSeNGp_tJPhIAuk4Iksk0pxM
```
```
$ chmod +x container.sh
$ ./container.sh run release-5g
$ sudo bin/lte_fuzzer  --EnableSimulator=true
```
## 📄 Summary of CVEs:
Currently, U-Fuzz has 11 CVEs available. The correspondence between the exploit name and
U-Fuzz vulnerability is shown in the Table below.

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

# 5. 📝 Citing U-Fuzz

```
@article{
  author={Shang, Zewen and Garbelini, Matheus E and Chattopadhyay, Sudipta},
  booktitle={2024 IEEE Conference on Software Testing, Verification and Validation (ICST)}, 
  title={U-Fuzz: Stateful Fuzzing of IoT Protocols on COTS Devices}, 
  year={2024},
}
```