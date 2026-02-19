#!/bin/bash

sudo systemctl daemon-reload
sudo systemctl start postgresql.service cassandra.service elasticsearch.service

echo "Services running"