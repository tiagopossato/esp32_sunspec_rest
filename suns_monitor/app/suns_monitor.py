"""
A simple example of how to get data from a SunSpec device with REST API.
"""
import requests
from requests.auth import HTTPBasicAuth
from threading import Thread
from save_data import start_db_session, save_data
from time import time, sleep
from datetime import datetime

requests.packages.urllib3.disable_warnings()
print("\nATENÇÃO, WARNINGS DESABILITADOS\n")

# variables to connect to the REST API
server_url = "https://192.168.1.27/v1/"
# parameters to get data
model = 307
instance = 0
points = "TmpAmb,RH,Pres"

# variables to calculate average request time
avg=0
count=0

# interval between requests
interval=5

if __name__ == '__main__':
    # start the database session
    session = start_db_session()
    # loop forever
    while(True):
        # get the initial datetime
        init_time = datetime.now()
        print(f"Requesting values at: {init_time}")
        try:
            # get the data over REST API
            r = requests.get(f"{server_url}models/{model}/instances/{instance}?points={points}", verify=False, timeout=5, auth=HTTPBasicAuth("admin", "admin"))
            # get the final datetime
            end_time = datetime.now()
            # calculate the request time
            request_time=(datetime.now() - init_time).total_seconds()
            # calculate the average request time
            count = count + 1
            avg = ((avg*(count-1))+request_time)/count
            print(f"Request time: {request_time:.4}s, avg: {avg:.4}s, count: {count}")

            # print the response code
            print(r.status_code)
            # get the json response
            data_points = r.json()['models'][0]
            
            # iterate over json keys and save data
            for key in data_points:
                # ignore 'name' and 'id' keys
                if key != 'name' and key != 'id':
                    save_data(model=model, instance=instance, point=key, value=data_points[key], timestamp=time(), session=session)
            # sleep until the next request
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
