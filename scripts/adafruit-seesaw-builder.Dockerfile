FROM ubuntu:xenial

RUN apt-get update && apt-get install -y \
    curl \
    libexpat1 \
    libisl15 \
    libmpc3 \
    libmpfr4 \
    python \
    libpython2.7 \
    make

RUN mkdir /debs

WORKDIR /debs

RUN curl -LO https://s3.amazonaws.com/adafruit-circuit-python/gcc-arm-embedded_7-2018q2-1~xenial1_amd64.deb

RUN echo "8c359c83adf69a8e3be27f239e14725bb000fdbc0b2f3dc14e9b7654dbcf89ce gcc-arm-embedded_7-2018q2-1~xenial1_amd64.deb" | sha256sum --check

RUN dpkg -i gcc-arm-embedded_7-2018q2-1~xenial1_amd64.deb
