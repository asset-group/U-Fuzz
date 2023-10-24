FROM ubuntu:18.04 as base

ENV HOME /root/
ENV TERM xterm-256color

WORKDIR $HOME

ADD configs/ configs/
ADD scripts/ scripts/
ADD libs/ libs/
ADD src/ src/
ADD CMakeLists.txt *.cmake requirements.sh $HOME

# Fix timezone for tzdata
RUN ln -snf /usr/share/zoneinfo/$CONTAINER_TIMEZONE /etc/localtime && echo $CONTAINER_TIMEZONE > /etc/timezone
RUN apt-get update && apt-get install sudo x11-xserver-utils -y && ./requirements.sh dev && rm ./* -rdf

CMD sleep infinity
