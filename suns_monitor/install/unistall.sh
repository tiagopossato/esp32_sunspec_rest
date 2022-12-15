#!/usr/bin/env bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

echo "Parando serviço"
systemctl stop suns_monitor.service
systemctl disable suns_monitor.service
rm /etc/systemd/system/suns_monitor.service

echo "removendo pasta"
rm -rf /opt/suns_monitor

echo "Concluído"
