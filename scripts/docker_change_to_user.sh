#!/usr/bin/env bash
GID_=$(id -g $USER)
UID_=$(id -u $USER)

# Receve command as arguments
CMD_ARG=$1
shift
CMD_FULL="$CMD_ARG $@"
echo $CMD_FULL

mkdir -p /home/$USER &> /dev/null
mkdir -p $PROJ_FOLDER
chown $USER /home/$USER
rm -d $PROJ_FOLDER
ln -s /root $PROJ_FOLDER
addgroup --gid $GID_ $USER &> /dev/null
useradd --home /home/$USER --gid $GID_ --uid $UID_  $USER &> /dev/null
cd $PROJ_FOLDER

# Start shell or execute command as user
if [ -z "$CMD_ARG" ]
then
	su $USER
else
	echo "$(which cmake)"
	su $USER -c "$CMD_FULL"
fi