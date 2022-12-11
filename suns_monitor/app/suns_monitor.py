import requests
from requests.auth import HTTPBasicAuth
from threading import Thread
from save_data import start_db_session, save_data
from time import time, sleep
from datetime import datetime

requests.packages.urllib3.disable_warnings()
print("\nATENÇÃO, WARNINGS DESABILITADOS\n")

server_url = "https://192.168.4.1/v1/"

avg=0
count=0
interval=5

if __name__ == '__main__':
    session = start_db_session()

    while(True):
        init_time = datetime.now()
        print(f"Requesting values at: {init_time}")
        try:
            r = requests.get(f"{server_url}models/307/instances/0?points=TmpAmb,RH,Pres", verify=False, timeout=5, auth=HTTPBasicAuth("admin", "admin"))
            end_time = datetime.now()
            request_time=(datetime.now() - init_time).total_seconds()
            count = count + 1
            avg = ((avg*(count-1))+request_time)/count
            print(f"Request time: {request_time:.4}s, avg: {avg:.4}s, count: {count}")
            print(r.status_code)

            resposta = r.json()

            print(resposta)

            save_data(model=307, instance=0, point="TmpAmb", value=resposta['models'][0]['TmpAmb'], timestamp=time(), session=session)
            save_data(model=307, instance=0, point="RH", value=resposta['models'][0]['RH'], timestamp=time(), session=session)
            save_data(model=307, instance=0, point="Pres", value=resposta['models'][0]['Pres'], timestamp=time(), session=session)

            try:
                sleep(interval-(end_time - init_time).total_seconds())
            except:
                sleep(.1)
        except Exception as e:
            print(e)
            try:
                sleep(interval-(datetime.now() - init_time).total_seconds())
            except:
                sleep(.1)
