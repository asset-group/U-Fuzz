#!/usr/bin/env bash

cd open5gs-core/webui
npm run dev &
sleep 5
google-chrome 127.0.0.1:3000
kill -9 $(pgrep node)
