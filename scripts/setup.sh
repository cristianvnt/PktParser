#!/bin/bash
set -e

if [ "$EUID" -eq 0 ]; then 
    echo "Error: Do not run as root"
    exit 1
fi

echo ">>> Installing build tools..."
sudo apt-get update
sudo apt-get install -y \
    build-essential gcc-13 g++-13 cmake git wget curl ninja-build \
    libjemalloc-dev libreadline-dev libffi-dev bison flex autoconf

echo ">>> Installing vcpkg..."
if [ ! -d "$HOME/vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
    "$HOME/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
    echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> "$HOME/.bashrc"
    echo 'export PATH="$VCPKG_ROOT:$PATH"' >> "$HOME/.bashrc"
    export VCPKG_ROOT="$HOME/vcpkg"
fi

echo ">>> Installing PostgreSQL..."
sudo apt-get install -y postgresql postgresql-contrib libpq-dev

echo ">>> Installing Java 17..."
sudo apt-get install -y openjdk-17-jdk maven

echo ">>> Installing Cassandra 5.0..."
sudo rm -f /etc/apt/sources.list.d/cassandra*
echo "deb [signed-by=/etc/apt/keyrings/apache-cassandra.asc] https://debian.cassandra.apache.org 50x main" | sudo tee /etc/apt/sources.list.d/cassandra.sources.list
sudo curl -o /etc/apt/keyrings/apache-cassandra.asc https://downloads.apache.org/cassandra/KEYS
sudo apt-get update
sudo apt-get install -y cassandra

echo ">>> nstalling Elasticsearch 9.x..."
if ! dpkg -l | grep -q '^ii.*elasticsearch'; then
    wget -qO - https://artifacts.elastic.co/GPG-KEY-elasticsearch | sudo gpg --dearmor -o /usr/share/keyrings/elasticsearch-keyring.gpg
    echo "deb [signed-by=/usr/share/keyrings/elasticsearch-keyring.gpg] https://artifacts.elastic.co/packages/9.x/apt stable main" | \
        sudo tee /etc/apt/sources.list.d/elastic-9.x.list
    sudo apt-get update
    sudo apt-get install -y elasticsearch

    sudo sed -i '/^xpack.security.enabled:/d' /etc/elasticsearch/elasticsearch.yml
    echo "xpack.security.enabled: false" | sudo tee -a /etc/elasticsearch/elasticsearch.yml
fi

echo ">>> Installing Python dependencies..."
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

echo "Setup complete. 'source ~/.bashrc' or restart terminal"