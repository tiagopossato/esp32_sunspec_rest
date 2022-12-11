# Arquivos para consumir a API com uma dashboard do Grafana

## Data source

- Habilitar o datasource https://grafana.com/grafana/plugins/yesoreyeram-infinity-datasource/
  - sudo grafana-cli plugins install yesoreyeram-infinity-datasource
  - sudo systemctl restart grafana-server.service
  - Configurar a aba "Authentication"
  - Em "TLS/SSL & Network Settings", habilitar "Skip TLS Verify"
   
- Habilitar o datasource SQLite
  - sudo grafana-cli plugins install frser-sqlite-datasource
  - sudo systemctl restart grafana-server.service


## Dashboards

- Importar a dashboard

- Por algum motivo ainda não descoberto, para funcionar depois de importar a dashboard, é preciso entrar em cada painel, fazer alguma alteração, reverter a alteração e salvar.