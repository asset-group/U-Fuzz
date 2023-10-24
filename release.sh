#!/usr/bin/env bash
./build.sh all
./requirements.sh doc
./build.sh doc
./build.sh release
./build.sh release_exploiter
# Build ESP32 BT Classic firmware
python3 src/drivers/firmware_bluetooth/firmware.py build
# Copy zipped firmware to release folder
cp src/drivers/firmware_bluetooth/release/esp32driver.zip release
# Clean CI/CD repo artifacts if environment is set
if [[ -v GITLAB_TOKEN ]]; then
	./build.sh clean_gitlab_artifacts
fi
