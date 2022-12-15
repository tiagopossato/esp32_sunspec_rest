import requests
from requests.auth import HTTPBasicAuth
from threading import Thread
from time import time, sleep
from datetime import datetime

requests.packages.urllib3.disable_warnings()
print("\nATENÇÃO, WARNINGS DESABILITADOS\n")

server_url = "https://192.168.1.27/v1/"

avg=0
count=0
def get():
    global avg
    global count
    init_time = datetime.now()
    print(f"Getting at: {init_time}")
    try:
        r = requests.get(f"{server_url}models/307/instances/0?points=RH,TmpAmb,Pres", verify=False, timeout=5, auth=HTTPBasicAuth("admin", "admin"))
        end_time = datetime.now()
        request_time=(datetime.now() - init_time).total_seconds()
        count = count + 1
        avg = ((avg*(count-1))+request_time)/count
        print(f"Request time: {request_time:.4}s, avg: {avg:.4}s, count: {count}")
        print(f'HTTP: {r.status_code} -> {r.json()}\n')
        
    except Exception as e:
        print(e)

if __name__ == '__main__':
    while(count<2):
        get()
        
