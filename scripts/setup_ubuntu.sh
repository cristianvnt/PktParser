#!/bin/bash
set -e

echo "=============================="
echo "WoW Pkt Parser - Ubuntu Setup"
echo "=============================="
echo ""

if [ "$EUID" -eq 0 ]; then 
    echo "Error: Do not run this script as root"
    exit 1
fi

echo "Updating package list..."
sudo apt-get update

# essentials
echo ""
echo "Installing build tools..."
sudo apt-get install -y \
    build-essential \
    gcc-11 \
    g++-11 \
    cmake \
    make \
    git \
    wget \
    curl

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

echo "Build tools installed"

# c++ libs
echo ""
echo "Installing C++ libraries..."
sudo apt-get install -y \
    libfmt-dev \
    libssl-dev \
    zlib1g-dev

echo "C++ libraries installed"

# psql
echo ""
echo "Installing PostgreSQL..."
sudo apt-get install -y \
    postgresql \
    postgresql-contrib \
    libpq-dev

sudo systemctl start postgresql
sudo systemctl enable postgresql

echo "PostgreSQL installed"

# cassandra
echo ""
echo "Installing Cassandra..."

sudo apt-get install -y openjdk-11-jdk

if ! dpkg -l | grep -q cassandra; then
    echo "Installing apt-transport-https..."
    sudo apt-get install -y apt-transport-https
    
    echo "Adding Cassandra repository..."
    
    # remove any existing Cassandra repository files first
    sudo rm -f /etc/apt/sources.list.d/cassandra*
    
    echo "deb https://debian.cassandra.apache.org 41x main" | sudo tee /etc/apt/sources.list.d/cassandra.list
    wget -qO- https://downloads.apache.org/cassandra/KEYS | sudo gpg --dearmor -o /usr/share/keyrings/cassandra-archive-keyring.gpg
    
    echo "Updating package lists..."
    sudo apt-get update
    
    echo "Installing Cassandra..."
    sudo apt-get install -y cassandra
else
    echo "Cassandra is already installed"
fi

sudo systemctl start cassandra
sudo systemctl enable cassandra

echo "Cassandra installed"

# cassandra c++ driver
echo ""
echo "Installing Cassandra C++ driver..."

sudo apt-get install -y libuv1-dev

DRIVER_DIR="/tmp/cassandra-cpp-driver"
if [ ! -d "$DRIVER_DIR" ]; then
    cd /tmp
    git clone https://github.com/datastax/cpp-driver.git cassandra-cpp-driver
    cd cassandra-cpp-driver
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    sudo make install
    sudo ldconfig
    echo "Cassandra C++ driver installed"
else
    echo "Cassandra C++ driver already exists"
fi

# python dependencies
echo ""
echo "Installing Python dependencies..."
sudo apt-get install -y python3 python3-pip python3-psycopg2 python3-dotenv

echo "Python dependencies installed"

##### VALIDATION #####

validation_ok=true

if gcc-11 --version >/dev/null 2>&1; then
    echo "GCC 11: OK"
else
    echo "GCC 11: MISSING"
    validation_ok=false
fi

if cmake --version >/dev/null 2>&1; then
    echo "CMake: OK"
else
    echo "CMake: MISSING"
    validation_ok=false
fi

if sudo -u postgres psql -c "SELECT 1" >/dev/null 2>&1; then
    echo "PostgreSQL: OK"
else
    echo "PostgreSQL: FAILED"
    validation_ok=false
fi

if nodetool status >/dev/null 2>&1; then
    echo "Cassandra: OK"
else
    echo "Cassandra: FAILED"
    validation_ok=false
fi

if python3 -c "import psycopg2" 2>/dev/null; then
    echo "Python psycopg2: OK"
else
    echo "Python psycopg2: MISSING"
    validation_ok=false
fi

echo ""
if $validation_ok; then
    echo "Setup completed successfully"
    echo ""
    exit 0
else
    echo "Setup completed with errors"
    exit 1
fi