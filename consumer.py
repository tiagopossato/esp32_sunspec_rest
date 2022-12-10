import requests
from requests.auth import HTTPBasicAuth
from threading import Thread

import csv
from datetime import datetime
from time import sleep

requests.packages.urllib3.disable_warnings()
print("\nATENÇÃO, WARNINGS DESABILITADOS\n")

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


# with open('resposta.csv', 'w', newline='') as csvfile:
#     fieldnames = ['datetime','userId', 'id']
#     writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
#     writer.writeheader()


# with open('resposta.csv', 'a', newline='') as csvfile:
#     fieldnames = ['datetime','userId', 'id']
#     writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

#     writer.writerow({'datetime':  datetime.now(), 'userId': resposta['userId'], 'id': resposta['id']})
        
server_url = "https://192.168.4.1/v1/"

def perform_request(uri, username, password, interval, times = True):
	avg=0
	count=0
	while(times):
		if(times is not True):
			times = times - 1
		
		init_time = datetime.now()
		print(f"Requesting values at: {init_time}")
		try:
			r = requests.get(f"{server_url}{uri}", verify=False, timeout=5, auth=HTTPBasicAuth(username, password))
			end_time = datetime.now()
			request_time=(datetime.now() - init_time).total_seconds()
			count = count + 1
			avg = ((avg*(count-1))+request_time)/count
			print(f"Request time: {request_time:.4}s, avg: {avg:.4}s, count: {count}")
			print(r.status_code)
			
			resposta = r.json()
			print(resposta)
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



num_threads = 10

if __name__ == "__main__":

	# lista de threads
	threads = []
	for i in range(0,num_threads):
		# cria objeto da thread
		t = Thread(target=perform_request, args=("models/307/instances/0?points=TmpAmb,RH,Pres","admin","admin",1))
		#inicia a thread
		t.start()
		# adiciona a thread na lista de threads
		threads.append(t)
		
	#aguarda o encerramento de todas as threads
	for i in range(0,num_threads):
		threads[i].join()
	
	print("Saindo...")
	
	
