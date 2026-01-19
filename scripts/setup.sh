#!/bin/bash
set -e

if [ "$EUID" -eq 0 ]; then 
    echo "Error: Do not run as root"
    exit 1
fi

echo "Installing build tools..."
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget curl

gcc --version
g++ --version

echo "Installing C++ libraries..."
sudo apt-get install -y libfmt-dev libssl-dev zlib1g-dev libuv1-dev

echo "Installing PostgreSQL..."
sudo apt-get install -y postgresql postgresql-contrib libpq-dev
sudo systemctl enable --now postgresql

echo "Installing Java 17..."
sudo apt-get install -y openjdk-17-jdk

echo "Installing Cassandra 5.0..."
sudo rm -f /etc/apt/sources.list.d/cassandra*
echo "deb [signed-by=/etc/apt/keyrings/apache-cassandra.asc] https://debian.cassandra.apache.org 50x main" | sudo tee /etc/apt/sources.list.d/cassandra.sources.list
sudo curl -o /etc/apt/keyrings/apache-cassandra.asc https://downloads.apache.org/cassandra/KEYS
sudo apt-get update
sudo apt-get install -y cassandra
sudo systemctl enable --now cassandra

echo "Installing Cassandra C++ driver..."
if [ ! -d "/tmp/cassandra-cpp-driver" ]; then
    cd /tmp
    git clone https://github.com/datastax/cpp-driver.git cassandra-cpp-driver
    cd cassandra-cpp-driver
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    sudo make install
    sudo ldconfig
fi

echo "Installing Python dependencies..."
sudo apt-get install -y python3-pip python3-psycopg2 python3-dotenv

echo "Setup complete"