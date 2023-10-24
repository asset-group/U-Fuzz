#### Bulding Core Network (EPC) + Base Station (eNB) + Using WDissector

##### 1) Install System Requirements

```bash
./requirements.sh
```

##### 2) Build Core Network

```bash
./build_core.sh
```

During build process, git will ask your email and password to clone from our private repository.

The script will download everything (including WDissector binary release) to your current working directory.

The following folders will be created:

* open5gs-core - Core network
* wdissector - WDissector binary release which includes lte_fuzzer

##### 3) Open Air Interface (OAI)

```bash
./build_enb.sh
```

The following folders will be created:

* oai_lte_network - Our customized Open Air Interface project (eNB)

##### 4) Configure projects

Apply the correct configuration for open5gs-core, wdissector and OAI

```bash
./configure_projects.sh
```

This will copy the configuration files from `configs` folder to respective projects

##### 5) (Optional) Open the core network webserver which holds the SIM cards credentials

```bash
./run_core_webserver.sh
```

This will start a webserver at 127.0.0.1:3000 and google-chrome will automatically open after 5 seconds.

#### 6) LTE Fuzzer

Connect USRP B210 SDR to your computer on any USB3.0 port and start the fuzzer as follows:

```bash
./run_lte_fuzzer.sh
```



### 6) Connect the UE (4G Module) to the network

1. Start `cutecom` and open serial port /dev/ttyACM0
2. Enable the checkbox DTR
3. Type the commands included in at_commands.txt accordingly.
4. Check lte_fuzzer shows connection with UE module. **If module does not connect, try reppluging the UE module from your computer or restart lte_fuzzer**
