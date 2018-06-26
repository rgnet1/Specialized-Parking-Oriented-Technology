import requests
import json

URL = '''http://car-management-66420.appspot.com/login'''
#URL = '''http://localhost:8080/login'''

data = {"email":"jokwlam@ucsc.edu", "password":"test"}

r = requests.post(url = URL, data = json.dumps(data))

for line in json.loads(r.text):
    print line + ' ' + json.loads(r.text)[line]
