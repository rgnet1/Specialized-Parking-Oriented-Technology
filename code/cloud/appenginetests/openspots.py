import requests
import json

#URL = '''http://car-management-66420.appspot.com/spots'''
URL = '''http://localhost:8080/spots'''

data = {"Parking_Lot":"EastRemote"}
headers = {'Message_Type': 'Spots'}

r = requests.get(url = URL, params = data, headers = headers)

for line in json.loads(r.text):
    print line + ' ' + json.loads(r.text)[line]
