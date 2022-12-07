import requests
import csv
from datetime import datetime
from time import sleep

# r = requests.get('https://jsonplaceholder.typicode.com/posts') #, verify=False
# print(r.status_code)
# if(r.status_code == 200):
#     print(r.json())


# parametros = {'userId':'1'}

# r = requests.get('https://jsonplaceholder.typicode.com/posts', params=parametros)
# print(r.url)
# print(r.status_code)
# if(r.status_code == 200):
#     print(r.json())


# payload = {'title': 'Tiago Possato', 'body': 'meu primeiro post', 'userId': 1}

# r = requests.post("https://jsonplaceholder.typicode.com/posts", data=payload)
# print(r.status_code)
# if(r.status_code == 201):
#     print(r.json())

# payload = {'title': 'Possato'}

# r = requests.patch("https://jsonplaceholder.typicode.com/posts/1", data=payload)
# print(r.status_code)
# print(r.json())
# """


with open('resposta.csv', 'w', newline='') as csvfile:
    fieldnames = ['datetime','userId', 'id']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()

while(True):
    try:
        # measure resquest time
        init_time = datetime.now()
        try:
            r = requests.get('https://jsonplaceholder.typicode.com/posts/1') #, verify=False
        except:
            print("Error")
            sleep(1)
            continue
        end_time = datetime.now()
        print(f"Request time: {end_time - init_time}")
        print(r.status_code)

        resposta = r.json()
        print(resposta)

        with open('resposta.csv', 'a', newline='') as csvfile:
            fieldnames = ['datetime','userId', 'id']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

            writer.writerow({'datetime':  datetime.now(), 'userId': resposta['userId'], 'id': resposta['id']})
                
        sleep(1)
    except:
        print("Error")
        sleep(1)