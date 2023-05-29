#!/bin/sh
cd /home
make server
bin/server &
python3 /home/host_metrics_app/main.py docker-compose.yml