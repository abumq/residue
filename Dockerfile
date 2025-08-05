FROM ubuntu:24.04 as builder

# Install build dependencies
RUN apt-get update && \
    apt-get install -y build-essential cmake libgtest-dev libssl-dev wget unzip g++ && \
    apt-get install -y zlib1g-dev zlib1g libcurl4-openssl-dev

# Install GTest 1.7.0
RUN cd && wget -O gtest.tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz && \
    tar xf gtest.tar.gz && \
    cd googletest-release-1.7.0 && \
    cmake -DBUILD_SHARED_LIBS=ON . && \
    make && \
    cp -a include/gtest /usr/include && \
    cp -a libgtest_main.so libgtest.so /usr/lib/

# Install Easylogging++
RUN cd && wget -O easyloggingpp.zip https://github.com/abumq/easyloggingpp/archive/master.zip && \
    unzip easyloggingpp.zip && \
    cd easyloggingpp-master && \
    cmake . && \
    make install

# Install Crypto++ 5.6.5
RUN cd && wget https://github.com/abumq/abumq.github.io/raw/refs/heads/main/downloads/cryptocpp.tar.gz && \
    tar xf cryptocpp.tar.gz && \
    cd cryptopp-CRYPTOPP_5_6_5 && \
    wget https://github.com/abumq/abumq.github.io/raw/refs/heads/main/downloads/pem_pack.zip && \
    unzip pem_pack.zip && \
    cmake . && \
    make CXXFLAGS="-std=c++11" && \
    make install

# Install Ripe
RUN cd && wget -O ripe.zip https://github.com/abumq/ripe/archive/master.zip && \
    unzip ripe.zip && \
    cd ripe-master && \
    mkdir build && \
    cd build && \
    cmake .. && \
    cmake -DBUILD_SHARED_LIBS=OFF .. && \
    make && \
    make install

# Copy source code
COPY . /residue
WORKDIR /residue

# Build Residue
RUN mkdir build && \
    cd build && \
    cmake -Dproduction=ON .. && \
    make

# Final image
FROM ubuntu:24.04

# Create a non-root user
RUN useradd -ms /bin/bash residue

RUN apt-get update && \
    apt-get install -y libcurl4 g++ && \
    rm -rf /var/lib/apt/lists/*

# Copy the binary from the builder stage
COPY --from=builder /residue/build/residue /usr/local/bin/residue

# Set the user
USER residue

# Set the entrypoint
ENTRYPOINT ["/usr/local/bin/residue"]