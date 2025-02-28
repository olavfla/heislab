FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y gcc make findutils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN make

CMD ["/app/elevator"]