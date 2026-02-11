#!/bin/bash
set -e

if [ "$EUID" -eq 0 ]; then 
    echo "Error: Do not run as root"
    exit 1
fi

echo "Installing build tools..."
sudo apt-get update
sudo apt-get install -y build-essential gcc-13 g++-13 cmake git wget curl ninja-build
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 110
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 110
sudo update-alternatives --auto gcc
sudo update-alternatives --auto g++

echo "Installing C++ libraries..."
sudo apt-get install -y libfmt-dev libssl-dev zlib1g-dev libuv1-dev libcurl4-openssl-dev

echo "Installing PostgreSQL..."
sudo apt-get install -y postgresql postgresql-contrib libpq-dev

echo "Installing Java 17..."
sudo apt-get install -y openjdk-17-jdk

echo "Installing Cassandra 5.0..."
sudo rm -f /etc/apt/sources.list.d/cassandra*
echo "deb [signed-by=/etc/apt/keyrings/apache-cassandra.asc] https://debian.cassandra.apache.org 50x main" | sudo tee /etc/apt/sources.list.d/cassandra.sources.list
sudo curl -o /etc/apt/keyrings/apache-cassandra.asc https://downloads.apache.org/cassandra/KEYS
sudo apt-get update
sudo apt-get install -y cassandra

echo "Installing Cassandra C++ driver..."
if [ ! -d "/tmp/cassandra-cpp-driver" ]; then
    cd /tmp
    rm -rf cassandra-cpp-driver
    git clone https://github.com/datastax/cpp-driver.git cassandra-cpp-driver
    cd cassandra-cpp-driver
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j4
    sudo make install
    sudo ldconfig
fi

echo "Installing Elasticsearch 9.x..."
if ! dpkg -l | grep -q elasticsearch; then
    wget -qO - https://artifacts.elastic.co/GPG-KEY-elasticsearch | sudo gpg --dearmor -o /usr/share/keyrings/elasticsearch-keyring.gpg
    echo "deb [signed-by=/usr/share/keyrings/elasticsearch-keyring.gpg] https://artifacts.elastic.co/packages/9.x/apt stable main" | sudo tee /etc/apt/sources.list.d/elastic-9.x.list
    sudo apt-get update
    sudo apt-get install -y elasticsearch

    # security disable for local dev - unnecessary complexity
    sudo sed -i '/^xpack.security.enabled:/d' /etc/elasticsearch/elasticsearch.yml
    echo "xpack.security.enabled: false" | sudo tee -a /etc/elasticsearch/elasticsearch.yml
fi

echo "Installing Python dependencies..."
sudo apt-get install -y pipx python3-pip python3-psycopg2 python3-dotenv
pipx ensurepath

echo "Installing cqlsh for Python 3.12..."
pipx install cqlsh
export PATH="$HOME/.local/bin:$PATH"

echo "Setup complete"