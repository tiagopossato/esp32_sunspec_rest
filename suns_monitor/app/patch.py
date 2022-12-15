import requests
from requests.auth import HTTPBasicAuth
from threading import Thread
from time import time, sleep
from datetime import datetime

requests.packages.urllib3.disable_warnings()
print("\nATENÇÃO, WARNINGS DESABILITADOS\n")

server_url = "http://192.168.4.1/v1/"

avg=0
count=0
def patch(data):
    global avg
    global count
    init_time = datetime.now()
    print(f"Patching {data} at: {init_time}")
    try:
        r = requests.patch(f"{server_url}models/1/instances/0", verify=False, timeout=5, json=data, auth=HTTPBasicAuth("admin", "admin"))
        end_time = datetime.now()
        request_time=(datetime.now() - init_time).total_seconds()
        count = count + 1
        avg = ((avg*(count-1))+request_time)/count
        print(f"Request time: {request_time:.4}s, avg: {avg:.4}s, count: {count}")
        print(f'HTTP: {r.status_code} -> {r.json()}\n')
        
    except Exception as e:
        print(e)

if __name__ == '__main__':
    # while(True):
# The PATCH method SHALL update all or partial Device Information Model data for the specified model instance of the specified model. All model updates are considered partial updates to an existing resource.

# Only one model instance can be updated at a time. The body of the PATCH request SHALL be encoded as a model populated with the points to be updated. Any point that is not present in the model provided in the body SHALL NOT be affected by the update.

# If a point that is marked as read-only or unimplemented is part of the update, the request SHALL fail and return an HTTP status code of 405 Method Not Allowed.

# If a {modelIndex} is not provided in the URI and there is more than one instance of the model id in the device, the request SHALL fail and return an HTTP status code of 400 Bad Request.

# If a requested update cannot be performed, the request SHALL fail with no updates performed on the information model.

# Model data that are not being updated SHALL be represented as empty objects.
    while(True):
        patch({"id":1, "points":[{"DA":count%255},{"SN":f"{count%500}-jkl"}]})
        patch({"id":1, "points":[{"DA":{}},{"SN":f"{count%500}-jkl"}]})
        patch({"id":1, "points":[{"DA":{}},{"SN":{}}]})
        patch({"id":1, "points":[{"DA":count%255},{"SN":f"{count%500}-jkl"}]})
        patch({"id":1, "points":[{"DA":30},{"SN":"jjkkjfjdpsldfslfl"}]})
        patch({"id":1, "points":[{"DA":{}}]})
        patch({"id":1, "points":[]})
        patch({"id":1, "points":[{"DA":3},{"SN":f"{count%500}-jkl"}, {"Vr":"56"}]})