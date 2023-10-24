#!/usr/bin/env bash

CURRENT_PATH=$(pwd)
SED_CMD="s|/home/matheus/5g/|${CURRENT_PATH}/|g"


cp -f configs/mme.yaml configs/mme_generated.yaml
cp -f configs/smf.yaml configs/smf_generated.yaml
cp -f configs/enb_config.json configs/enb_config_generated.json
sed -i $SED_CMD configs/mme_generated.yaml
sed -i $SED_CMD configs/smf_generated.yaml
sed -i $SED_CMD configs/enb_config_generated.json

cp -f configs/enb_config_generated.json wdissector/configs/enb_config.json
cp -f configs/b7.conf wdissector/configs/lte_enb/b7.conf
cp -f configs/mme_generated.yaml open5gs-core/install/etc/open5gs/mme.yaml
cp -f configs/smf_generated.yaml open5gs-core/install/etc/open5gs/smf.yaml
