ARG VERSION
FROM ubuntu:${VERSION}

ENV DEBIAN_FRONTEND=noninteractive

COPY ./0xe1dd270288b4e6030699e45fa1715d88e1df1f24.gpg /etc/apt/trusted.gpg.d/

RUN : && \
# Instead of add-apt-repository and a two-stage apt-get
    . /etc/lsb-release && \
    echo "deb http://ppa.launchpadcontent.net/git-core/ppa/ubuntu ${DISTRIB_CODENAME} main" >> /etc/apt/sources.list && \
    apt-get update && \
# tzdata, even in ubuntu:20.04
    echo "tzdata tzdata/Areas select Etc" | debconf-set-selections && \
    echo "tzdata tzdata/Zones/Etc select UTC" | debconf-set-selections && \
    apt-get install -y \
# Due to Github's absurd requirement: https://github.com/actions/checkout
# > When Git 2.18 or higher is not in your PATH, falls back to the REST API to download the files.
      git \
# Only needed for Ubuntu Xenial
      dh-autoreconf \
      devscripts equivs lsb-release && \
    rm -rf /var/lib/apt/lists/*
