#!/bin/bash
set -e

if [ "$EUID" -eq 0 ]; then 
    echo "Error: Do not run as root"
    exit 1
fi

echo "Installing build tools..."
sudo apt-get update
sudo apt-get install -y build-essential gcc-13 g++-13 cmake git wget curl ninja-build

echo "Installing C++ libraries..."
sudo apt-get install -y libssl-dev zlib1g-dev libuv1-dev libcurl4-openssl-dev libreadline-dev libffi-dev libzstd-dev \
    libjsoncpp-dev libc-ares-dev libbrotli-dev uuid-dev

if [ ! -d "/tmp/fmt-build" ]; then
    cd /tmp
    rm -rf fmt fmt-build
    git clone https://github.com/fmtlib/fmt.git
    cd fmt
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j4
    sudo make install
    sudo ldconfig
    cd /tmp
    mv fmt fmt-build
fi

echo "Installing Drogon web framework..."
if [ ! -d "/tmp/drogon-build" ]; then
    cd /tmp
    rm -rf drogon drogon-build
    git clone https://github.com/drogonframework/drogon.git
    cd drogon
    git submodule update --init
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j4
    sudo make install
    sudo ldconfig
    cd /tmp
    mv drogon drogon-build
fi

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
if ! dpkg -l | grep -q '^ii.*elasticsearch'; then
    wget -qO - https://artifacts.elastic.co/GPG-KEY-elasticsearch | sudo gpg --dearmor -o /usr/share/keyrings/elasticsearch-keyring.gpg
    echo "deb [signed-by=/usr/share/keyrings/elasticsearch-keyring.gpg] https://artifacts.elastic.co/packages/9.x/apt stable main" | sudo tee /etc/apt/sources.list.d/elastic-9.x.list
    sudo apt-get update
    sudo apt-get install -y elasticsearch

    sudo sed -i '/^xpack.security.enabled:/d' /etc/elasticsearch/elasticsearch.yml
    echo "xpack.security.enabled: false" | sudo tee -a /etc/elasticsearch/elasticsearch.yml
fi

echo "Installing Python dependencies..."
sudo apt-get install -y pipx python3-pip python3-psycopg2 python3-dotenv

if [ ! -d "$HOME/.pyenv" ]; then
    curl https://pyenv.run | bash
fi

export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"

pyenv install 3.11.11 -s
pipx install cqlsh --python "$PYENV_ROOT/versions/3.11.11/bin/python3"
pipx ensurepath

sudo ln -sf "$HOME/.local/bin/cqlsh" /usr/local/bin/cqlsh

echo "Setup complete"