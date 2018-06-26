import requests
import json


URL = '''https://car-management-66420.appspot.com/gateway'''
#URL = '''http://localhost:8080/gateway'''

headers = {'Message_Type': 'Update', 'Gateway': '1'}

jsonData = {}
f = open('info.txt', 'r')

for line in f:
    temp = line.split()
    jsonData[temp[0]] = temp[1]

f.close()

for line in jsonData:
    print line + ' ' + jsonData[line]

r = requests.post(url = URL, data = json.dumps(jsonData), headers = headers)
response = json.loads(r.text)

for line in response:
    print line + ' ' + response[line]
