import requests
import json

URL = '''http://car-management-66420.appspot.com/parking_map'''
#URL = '''http://localhost:8080/parking_map'''

data = {"Parking_Lot":"EastRemote","Spots":"1"}

r = requests.get(url = URL, params = data)

for line in json.loads(r.text):
    print line + ' ' + json.loads(r.text)[line]

