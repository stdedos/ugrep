# step 1: create a container for ugrep named "ugrep"
# docker -D build -t ugrep .
#
# step 2: run bash in the container, e.g. to run ugrep from the command line
# docker run -it ugrep bash
#
# step 3: run ugrep in the container, for example:
# ugrep -r -n -tjava Hello ugrep/tests/
#
# step 4: extract Linux artifact (Works on >= Bionic 18.04)
# docker cp "$(docker run ugrep:latest hostname)":/ugrep/bin/ugrep .
# (or just step 2)

FROM ubuntu
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    make \
    g\+\+ \
    wget \
    autoconf \
    unzip \
    libpcre2-dev \
    libz-dev \
    libbz2-dev \
    liblzma-dev \
    liblz4-dev

RUN mkdir /ugrep

ADD . /ugrep

RUN cd /ugrep &&\
    autoreconf -fi &&\
    ./build.sh &&\
    make install
