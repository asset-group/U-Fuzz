#!/usr/bin/env bash

echo -e "\n\nconst char *default_config = R\"/($(cat $1))/\";" >> $2
