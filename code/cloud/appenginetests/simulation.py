import requests
import json
import sys
import time

#parkingspot, userid, bt_string

URL = '''http://car-management-66420.appspot.com/gateway'''
#URL = '''http://localhost:8080/gateway'''
data = {}
headers = {}
data[sys.argv[1]] = 'busy'
headers = {'Message_Type': 'Update', 'Gateway' : '1'}
r = requests.post(url = URL, data = json.dumps(data), headers = headers)
response = json.loads(r.text)
print 'Stage 1 done.....'
for line in response:
    print line + ' ' + response[line]

URL = '''http://car-management-66420.appspot.com/mobileapp'''
#URL = '''http://localhost:8080/mobileapp'''
data = {}
headers = {}
data = {'userid': sys.argv[2], 'bt_string': sys.argv[3]}
headers = {'Message_Type': "Verify"}
r = requests.post(url = URL, data = json.dumps(data), headers = headers)
response = json.loads(r.text)
print '\nStage 2 done.....'
for line in response:
    print line + ' ' + response[line]

if response['status'] == 'busy':
    print 'Spot not busy, exiting...'
    sys.exit()
    
if response['status'] == 'payment':
    URL = '''http://car-management-66420.appspot.com/payment'''
    #URL = '''http://localhost:8080/payment'''
    data = {}
    headers = {}
    data = {'userid': sys.argv[2], 'bt_string': sys.argv[3]}
    headers = {'Message_Type': "Payment"}
    r = requests.post(url = URL, data = json.dumps(data), headers = headers)
    response = json.loads(r.text)
    print '\nPayment complete:'
    for line in response:
        print line + ' ' + response[line]

URL = '''http://car-management-66420.appspot.com/gateway'''
#URL = '''http://localhost:8080/gateway'''
headers = {}
headers = {'Message_Type': 'Update', 'Gateway': '1'}
r = requests.get(url = URL, headers = headers)
response = json.loads(r.text)
print '\nStage 3 done.....'
for line in response:
    print line + ' ' + response[line]

n = raw_input("\nPress Enter to continue...\n")

URL = '''http://car-management-66420.appspot.com/gateway'''
#URL = '''http://localhost:8080/gateway'''
data = {}
headers = {}
data[sys.argv[1]] = 'open'
headers = {'Message_Type': 'Update', 'Gateway': '1'}
r = requests.post(url = URL, data = json.dumps(data), headers = headers)
response = json.loads(r.text)
print '\nStage 4 done.....'
for line in response:
    print line + ' ' + response[line]
