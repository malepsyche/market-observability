FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    pkg-config \
    librdkafka-dev \
    libboost-system-dev
WORKDIR /app
COPY . /app
RUN cmake . && make
CMD ["./kafka_producer"]
