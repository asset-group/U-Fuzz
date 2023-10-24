#!/usr/bin/env bash

node server.js &
PID1=$!

for i in {1..10}
do
   node client.js
   sleep 0.1
done

kill $PID1
