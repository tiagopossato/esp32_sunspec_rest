#!/usr/bin/env bash
user=""

# Verifica se está executando com root
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# Verifica o usário para as permissões de instalação
function verifyUser(){
  testuser=2
  if [ $# -ge 1 ]; then
    while [ $testuser -ge 2 ]
    do
        user=$1
        getent passwd $user > /dev/null
        testuser=$?
        if [ $testuser -eq 0 ]; then
          echo "Installing with user $user"
        else
          echo "The user does not exist, try again"
          verifyUser
        fi
    done
  else
    echo -n "User: "
    read user
    verifyUser $user
  fi
}

verifyUser $1


# Verifica se já existe uma instalação
if [ -d /opt/suns_monitor ]; then
    if [ -d /opt/suns_monitor/app ]; then
        # Remove a instalação anterior
        rm -rf /opt/suns_monitor/app
    fi
else
    mkdir /opt/suns_monitor
fi

# sudo apt install python3 python3-pip python3-venv

# Copia a pasta do app
cp -r ../app /opt/suns_monitor/app

cp uninstall.sh /opt/suns_monitor/uninstall.sh

# Verifica se não existe o banco de dados
if [ ! -d /opt/suns_monitor/db ]; then
    # cria pasta do banco de dados
    mkdir /opt/suns_monitor/db
else
    echo ""
    echo "WARNING"
    echo "------The database already exists. Please verify if the migration is necessary------"
    echo ""
fi

# Cria o ambiente virtual
# python3 -m venv /opt/suns_monitor/virtualEnv

#ativa o venv dashEnv
# source /opt/suns_monitor/virtualEnv/bin/activate

#install as dependencias
# pip install -r requirements.txt 

# Altera as permissões de acesso à pasta do app
chown -R ${user}:${user} /opt/suns_monitor

# Copia o arquivo do serviço suns_monitor-save
cp suns_monitor.service /etc/systemd/system/suns_monitor.service
sed -i 's/#user#/'$user'/g' /etc/systemd/system/suns_monitor.service

echo "Atualizando script de inicialização"
chmod 644 /etc/systemd/system/suns_monitor.service
systemctl enable suns_monitor.service
systemctl restart suns_monitor.service
