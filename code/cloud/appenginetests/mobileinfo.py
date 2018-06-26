import requests
import json

URL = '''http://car-management-66420.appspot.com/mobileinfo'''
#URL = '''http://localhost:8080/mobileinfo'''

data = {"email":"jokwlam@ucsc.edu","password":"test"}

r = requests.get(url = URL, params = data)

for line in json.loads(r.text):
    print line + ' ' + json.loads(r.text)[line]

