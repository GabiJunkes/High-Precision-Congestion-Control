FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    build-essential \
    wget \
    curl \
    git \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev \
    flex \
    bison \
    texinfo \
    python2

WORKDIR /tmp/gcc5

RUN wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/gcc-5-base_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/i/isl-0.18/libisl15_0.18-4_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/cpp-5_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/libasan2_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/libmpx0_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/libgcc-5-dev_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/gcc-5_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/libstdc++-5-dev_5.5.0-12ubuntu1_amd64.deb && \
    wget -c http://archive.ubuntu.com/ubuntu/pool/universe/g/gcc-5/g++-5_5.5.0-12ubuntu1_amd64.deb && \
    apt install -y ./g++-5_5.5.0-12ubuntu1_amd64.deb ./gcc-5_5.5.0-12ubuntu1_amd64.deb \
                  ./gcc-5-base_5.5.0-12ubuntu1_amd64.deb ./cpp-5_5.5.0-12ubuntu1_amd64.deb \
                  ./libisl15_0.18-4_amd64.deb ./libgcc-5-dev_5.5.0-12ubuntu1_amd64.deb \
                  ./libasan2_5.5.0-12ubuntu1_amd64.deb ./libmpx0_5.5.0-12ubuntu1_amd64.deb \
                  ./libstdc++-5-dev_5.5.0-12ubuntu1_amd64.deb

# Use gcc-5 por padrão
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50

# Preparar diretório de trabalho para o HPCC
WORKDIR /hpcc
RUN rm -rf /tmp/gcc5
