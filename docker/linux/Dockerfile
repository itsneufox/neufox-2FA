FROM ubuntu:22.04

# Build arguments for architecture and SSL configuration
ARG ARCH=x86
ARG SHARED_OPENSSL=0

ENV DEBIAN_FRONTEND=noninteractive

RUN \
    apt-get update && \
    apt-get install -y \
        cmake \
        ninja-build \
        clang-14 \
        python3-pip \
        wget \
        git \
    && \
    # Install multilib for x86/x86_64 cross-compilation
    if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x86_64" ]; then \
        dpkg --add-architecture i386 && \
        apt-get update && \
        apt-get install -y \
            gcc-multilib \
            g++-multilib \
            libstdc++-11-dev:i386; \
    fi && \
    # Install ARM cross-compilation tools
    if [ "$ARCH" = "arm32" ]; then \
        apt-get install -y \
            gcc-arm-linux-gnueabihf \
            g++-arm-linux-gnueabihf; \
    fi && \
    if [ "$ARCH" = "aarch64" ]; then \
        apt-get install -y \
            gcc-aarch64-linux-gnu \
            g++-aarch64-linux-gnu; \
    fi && \
    useradd -m builder && \
    su builder -c 'pip3 install --user conan' && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

USER builder
WORKDIR /build

ENV CC=/usr/bin/clang-14 \
    CXX=/usr/bin/clang++-14 \
    PATH=/home/builder/.local/bin:${PATH} \
    ARCH=${ARCH} \
    SHARED_OPENSSL=${SHARED_OPENSSL}

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
