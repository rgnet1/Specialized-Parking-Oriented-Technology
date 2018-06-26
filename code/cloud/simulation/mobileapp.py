import requests
import json
import time
import sys

userid = sys.argv[1]
bt_string = sys.argv[2]
sleep_time = sys.argv[3]

URL = '''https://car-management-66420.appspot.com/mobileapp'''
headers = {'Message_Type': 'Verify'}
data = {'userid': userid, 'bt_string': bt_string}

time.sleep(int(sleep_time))

r = request.post(url = URL, data = json.dumps(data), headers = headers)

response = r.json()
