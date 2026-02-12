#!/bin/bash

systemctl restart postgresql.service cassandra.service elasticsearch.service

echo "Services restarted"