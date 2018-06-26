import requests
import json


#URL = '''http://car-management-66420.appspot.com/gateway'''
URL = '''http://localhost:8080/gateway'''

headers = {'Message_Type': 'Update', 'Gateway': '1'}

r = requests.get(url = URL, headers = headers)

print r.text
