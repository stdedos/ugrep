# step 1: create a container for ugrep named "ugrep"
# docker -D build --no-cache -t ugrep .
#
# step 2: run bash in the container, e.g. to run ugrep from the command line
# docker run -it ugrep bash
#
# step 3: run ugrep in the container, for example:
# ugrep -r -n -tjava Hello ugrep/tests/
#
# *This*, or get the executable out with the original version, using:
# docker cp "$(docker run ugrep:latest hostname)":/ugrep/bin/ugrep .

FROM ubuntu
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    make \
    vim \
    git \
    g\+\+ \
    wget \
    unzip \
    libpcre2-dev \
    libz-dev \
    libbz2-dev \
    liblzma-dev \
    liblz4-dev
