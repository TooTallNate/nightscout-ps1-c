#!/bin/bash
mkdir -p packed
version="$(git describe --tags)"

make clean
docker build -t nightscout-ps1-build:alpine -f Dockerfile.alpine .
docker run -it --rm -v $PWD:/nightscout-ps1 nightscout-ps1-build:alpine
mv -v packed/nightscout-ps1 "packed/nightscout-ps1-v${version}-alpine-x64"

make clean
docker build -t nightscout-ps1-build:debian -f Dockerfile.debian .
docker run -it --rm -v $PWD:/nightscout-ps1 nightscout-ps1-build:debian
mv -v packed/nightscout-ps1 "packed/nightscout-ps1-v${version}-linux-x64"

make clean
make build
mv -v out/Default/nightscout-ps1 "packed/nightscout-ps1-v${version}-macos-x64"
