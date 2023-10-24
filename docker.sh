#!/usr/bin/env bash

CONATINER_NAME=wdissector
CONATINER_REPO=registry.gitlab.com/asset-sutd/software/wireless-deep-fuzzer

start_container(){
	docker rm $CONATINER_NAME &> /dev/null
	sudo xhost local:root &> /dev/null # allow xhost on host root
	docker run -ti -d --privileged --name $CONATINER_NAME \
	-e USER=$USER -e PROJ_FOLDER=$(pwd) \
	-e DISPLAY=$DISPLAY \
	-v /tmp/.X11-unix:/tmp/.X11-unix \
	-v /etc/passwd:/etc/passwd \
	-v /etc/shadow:/etc/shadow \
	-v /etc/sudoers:/etc/sudoers \
	--mount type=bind,source="$(pwd)"/,target=/root $CONATINER_NAME &> /dev/null
}

start_container_release(){
	sudo xhost local:root &> /dev/null # allow xhost on host root
	echo $CONATINER_NAME:release
	docker run -ti -d --privileged --network=host --name ${CONATINER_NAME}_release \
	-v /dev:/dev $CONATINER_NAME:release
}


if [ "$1" == "build" ]
then
	if [ "$2" == "dev" ]
	then
		TAG_NAME=${CONATINER_NAME}:latest
		IMG_NAME=${CONATINER_NAME}_latest
		sudo docker build --squash --compress -t $TAG_NAME .
	elif [ "$2" == "release" ]
	then
		TAG_NAME=${CONATINER_NAME}:release
		IMG_NAME=${CONATINER_NAME}_release
		sudo docker build --squash --compress -t $TAG_NAME -f scripts/docker_minimal.docker .
	else
		echo "missing build argument. use ./docker.sh build <dev or release> [export]"
	fi

	if [ "$3" == "export" ]
	then
		mkdir -p release
		docker image save $TAG_NAME | gzip -9 -c > release/$IMG_NAME.tar.gz
		chmod a+rw release/$IMG_NAME.tar.gz
		echo "Image release/$IMG_NAME.tar.gz created!"
	fi

elif [ "$1" == "start" ]
then
	start_container

elif [ "$1" == "stop" ]
then
	if [ "$2" == "release" ]
	then
		docker rm --force ${CONATINER_NAME}_release
	else
		docker rm --force $CONATINER_NAME
	fi

elif [ "$1" == "compile" ]
then
	start_container
	docker exec $CONATINER_NAME scripts/docker_change_to_user.sh ./build.sh all # Compile library

elif [ "$1" == "push" ]
then
	if [ "$2" == "release" ]
	then
		docker image tag wdissector:release ${CONATINER_REPO}:release
		docker push ${CONATINER_REPO}:release
	else
		docker image tag wdissector ${CONATINER_REPO}:dev
		docker push ${CONATINER_REPO}:dev
	fi

elif [ "$1" == "release" ]
then
	docker rm --force $CONATINER_NAME &> /dev/null
	start_container
	docker exec $CONATINER_NAME scripts/docker_change_to_user.sh ./release.sh # Compile library and create release

elif [ "$1" == "release_exploiter" ]
then
	docker exec $CONATINER_NAME scripts/docker_change_to_user.sh ./build.sh release_exploiter # Compile library and create release

elif [ "$1" == "shell" ]
then
	if [ "$2" == "release" ]
	then
		start_container_release || true
		docker exec -ti ${CONATINER_NAME}_release bash
	else
		start_container || true
		docker exec -ti $CONATINER_NAME scripts/docker_change_to_user.sh # Start container with bash and mount files
	fi

elif [ "$1" == "reshell" ]
then
	if [ "$2" == "release" ]
	then
		docker rm --force ${CONATINER_NAME}_release &> /dev/null
		start_container_release || true
		docker exec -ti ${CONATINER_NAME}_release bash
	else
		docker rm --force $CONATINER_NAME &> /dev/null
		start_container
		docker exec -ti $CONATINER_NAME scripts/docker_change_to_user.sh # Start container with bash and mount files
	fi

elif [ "$1" == "load" ]
then
	echo "Loading docker image"
	docker load --input $2

elif [ "$1" == "clean" ]
then
	docker rm --force $CONATINER_NAME
	docker rmi -f $CONATINER_NAME # Remove container image
	docker system prune --force
else
	echo "-------------- HELP ------------------"
	echo "---------  USER Commands -------------"
	echo "sudo ./docker compile                     - Build project via container in current folder"
	echo "sudo ./docker shell                       - Start docker container shell in current folder"
	echo "sudo ./docker reshell                     - Restart docker container and start shell in current folder"
	echo "sudo ./docker start                       - Start docker container"
	echo "sudo ./docker stop                        - Stop docker container"
	echo "sudo ./docker clean                       - Stop and remove/clean docker container and image"
	echo "sudo ./docker load <image path>           - Load $CONATINER_NAME docker image (.tar.gz)"
	echo "---------  Dev. Commands -------------"
	echo "sudo ./docker build dev                   - Build development docker container"
	echo "sudo ./docker build release               - Build docker container and create compressed image for release"
	echo "sudo ./docker release              		- Build and release binary distribution release/$CONATINER_NAME.tar.zst"
fi

# TODO: nvidia driver
# sudo add-apt-repository ppa:graphics-drivers/ppa
# sudo apt install nvidia-driver-450 -y
