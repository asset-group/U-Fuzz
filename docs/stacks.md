# :email: Protocol Stacks

WDissector is built upon well known protocols stack implementation. These are used to generate messages and to guide the target device towards a set of protocol procedures which are expected to be tested again unknown or insecure behaviour.

## Bluetooth Classic

### Implementation:

* Layer 1-2:
  * Baseband & LMP - [ESP32 Bluetooth Library](https://github.com/espressif/esp32-bt-lib) (Reverse Engineered)
* Layer 2-7:
  * L2CAP and profiles - [BlueKitchen](https://github.com/bluekitchen/btstack) (Open Source)

![BT Stack](./figures/stacks/bt_stack.svg)



## 4G / LTE

### Implementation:

* Layer 1-2:

  *  eNB (Base-station) - [Open Air Interface](https://gitlab.eurecom.fr/oai/openairinterface5g/-/tree/develop) (Open Source)

* Layer 3 -7:

  * Core Network - [Open5GS](https://github.com/open5gs/open5gs) (Open Source)

  

#### Control Plane

![4g](./figures/stacks/4g.jpg)

#### User Plane

![4g_user](./figures/stacks/4g_user.jpg)