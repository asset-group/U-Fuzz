#!/usr/bin/env bash

if [ -f "/etc/arch-release" ]
then
    # source perl binaries
    source /etc/profile.d/perlbin.sh
fi

if [ "$1" == "all" ]
then
	sudo rm build -rdf
    mkdir build
    cmake -B build -DDEV=TRUE -DWIRESHARK_GUI=TRUE
    ninja -C build
elif [ "$1" == "dev" ]
then
    cmake -B build -DDEV=TRUE
    ninja -C build
elif [ "$1" == "exploiter" ]
then
    EXPLOIT_BUILD=1 cmake -B build -DDEV=TRUE -DWIRESHARK_GUI=TRUE
    ninja -C build
elif [ "$1" == "rebuild" ]
then
    ninja -C build clean
    ninja -C build
elif [ "$1" == "clean" ]
then
    ninja -C build clean
elif [ "$1" == "release" ]
then
	echo "Creating compressed release/wdissector.tar.zst ..."
    WS_HEADERS=$(find ./libs/wireshark -name "*.h")
    CONFIGS_HEADERS=$(find ./src/configs -name "*.h*")
    mkdir -p release
    tar -I 'zstd -c -T0 --ultra -20 -' --exclude '*__pycache__*' -cf release/wdissector.tar.zst --transform 'flags=r;s|^|wdissector/|' \
    README.md icon.png imgui.ini requirements.sh bin/ modules/ configs/ bindings/ scripts/python_env.sh scripts/server_test.py \
    src/wdissector.h src/ModulesInclude.hpp src/ModulesStub.cpp libs/json.hpp Zigbee/ $CONFIGS_HEADERS $WS_HEADERS \
    3rd-party/hostapd/ 3rd-party/adb/ src/drivers/wifi/rtl8812au/ examples/ zigbee_dongle_connection
    echo "Done!"
elif [ "$1" == "release_exploiter" ]
then
    echo "Creating compressed release/wdexploiter.tar.zst ..."
    WS_HEADERS=$(find ./libs/wireshark -name "*.h")
    CONFIGS_HEADERS=$(find ./src/configs -name "*.h*")
    mkdir -p release
    tar -I 'zstd -c -T0 --ultra -20 -' --exclude '*__pycache__*' --exclude '*cef*' --exclude '*modules/python*' --exclude '*modules/server*' \
    --exclude '*modules/webview*' --exclude '*modules/modem_manager*' --exclude '*modules/requirements.txt' --exclude '*fuzzer*' --exclude '*chrome*' \
    --exclude '*bin/locales*'  --exclude '*bin/swiftshader*' --exclude '*pagmo*' --exclude '*GLES*' \
    -cf release/wdexploiter.tar.zst --transform 'flags=r;s|^|wdexploiter/|' \
    icon.png imgui.ini requirements.sh bin/ modules/ configs/ \
    src/wdissector.h src/ModulesInclude.hpp src/ModulesStub.cpp libs/json.hpp $CONFIGS_HEADERS $WS_HEADERS \
    3rd-party/hostapd/ 3rd-party/adb/ src/drivers/wifi/rtl8812au/
    echo "Done!"
elif [ "$1" == "cicd" ]
then
    scripts/gitlab_test_ci.sh
elif [ "$1" == "doc" ]
then
    cd docs/old_greyhound
    mkdir -p ../.vuepress/public/old_greyhound
    node shins.js --inline --output ../.vuepress/public/old_greyhound/index.html
    cp -r source ../.vuepress/public/old_greyhound/
    cp -r app ../.vuepress/public/old_greyhound/
    cd ../../
    npm run build
    npm run pdf
elif [ "$1" == "doc_serve" ]
then
    cd docs/old_greyhound
    mkdir -p ../.vuepress/public/old_greyhound
    node shins.js --inline --output ../.vuepress/public/old_greyhound/index.html
    cp -r source ../.vuepress/public/old_greyhound/
    cp -r app ../.vuepress/public/old_greyhound/
    cd ../../
    npm run build_only
elif [ "$1" == "clean_gitlab_artifacts" ]
then
	npm install node-fetch
	node scripts/clean_gitlab_artifacts.js
else
    if [ ! -f "build/CMakeCache.txt" ]
    then
        cmake -B build -DDEV=TRUE -DWIRESHARK_GUI=TRUE
    fi
    ninja -C build
fi
