import requests
import json

#URL = '''http://car-management-66420.appspot.com/payment'''
URL = '''http://localhost:8080/payment'''

data = {"userid":"1427605", "bt_string":"124512451245"}
headers = {'Message_Type': 'Payment'}

r = requests.post(url = URL, data = json.dumps(data), headers = headers)

for line in json.loads(r.text):
    print line + ' ' + json.loads(r.text)[line]
