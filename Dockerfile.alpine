# docker build -t nightscout-ps1-build:alpine -f Dockerfile.alpine .
# docker run -it --rm -v $PWD:/nightscout-ps1 nightscout-ps1-build:alpine
FROM alpine:edge

RUN apk add --no-cache \
  build-base \
  python

WORKDIR /nightscout-ps1

CMD make build && cp -v ./out/Default/nightscout-ps1 ./packed/ && make clean
