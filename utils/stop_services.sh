#!/bin/bash

sudo systemctl stop postgresql.service cassandra.service elasticsearch.service

echo "Services stopped"