import requests
import json
import sys

#URL = '''http://car-management-66420.appspot.com/mobileapp'''
argLen = len(sys.argv)
print "arg length is",str(argLen)
if argLen != 3:
	print "error"
	sys.exit()

URL = '''http://localhost:8080/mobileapp'''

data = {"userid":str(sys.argv[1]), "bt_string":str(sys.argv[2])}

print data
headers = {'Message_Type': 'Verify'}

r = requests.post(url = URL, data = json.dumps(data), headers = headers)

for line in json.loads(r.text):
    print line + ' ' + json.loads(r.text)[line]
