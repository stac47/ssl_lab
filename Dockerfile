FROM debian:latest

RUN apt-get update \
    && apt-get install -y \
        openssl \
        libssl-dev \
        build-essential \
    && apt-get clean

RUN mkdir -p /reproducer

WORKDIR /reproducer

COPY . /reproducer/

RUN make && ./main
