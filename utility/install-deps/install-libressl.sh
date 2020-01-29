#!/bin/sh

mkdir tmp
cd tmp

VERSION=3.0.2

#############################################
## download openssl-$VERSION

wget https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/openssl-$VERSION.tar.gz

#############################################
## clean dir

rm -rf openssl-$VERSION

#############################################
## unpack

tar -xvzf openssl-$VERSION.tar.gz
cd openssl-$VERSION

#############################################
## build and install openssl

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install
