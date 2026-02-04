FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config git curl ca-certificates \
    libssl-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev libboost-program-options-dev \
    libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev nlohmann-json3-dev libfmt-dev libhttp-parser-dev \
    && rm -rf /var/lib/apt/lists/*

# GoogleTest: build from source to avoid platform packaging issues
RUN apt-get update && apt-get install -y libgtest-dev cmake && rm -rf /var/lib/apt/lists/* \
  && cd /usr/src/gtest && cmake CMakeLists.txt && make && cp *.a /usr/lib/ || true

WORKDIR /workspace

# Copy entire repo into container
COPY . /workspace

# Build
RUN mkdir -p /workspace/build && cmake -S /workspace -B /workspace/build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build /workspace/build -j$(nproc)

# Default command: run tests
CMD ["ctest", "--test-dir", "/workspace/build", "--output-on-failure"]
