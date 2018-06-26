#!/usr/bin/env python

import webapp2
import cgi
import os
from google.appengine.ext.webapp import template
import MySQLdb
import json
import time
import jinja2

CLOUDSQL_CONNECTION_NAME = os.environ.get('CLOUDSQL_CONNECTION_NAME')
CLOUDSQL_USER = os.environ.get('CLOUDSQL_USER')
CLOUDSQL_PASSWORD = os.environ.get('CLOUDSQL_PASSWORD')
CLOUDSQL_DATABASE = os.environ.get('CLOUDSQL_DATABASE')

JINJA_ENVIRONMENT = jinja2.Environment(
    loader=jinja2.FileSystemLoader(os.path.dirname(__file__)),
    extensions=['jinja2.ext.autoescape'],
    autoescape=True)

##### Database function calls #####
def connect_to_cloudsql():
    """
    Connects the program to the mySQL database
    
    """
    try:
        db = MySQLdb.connect(
            host = CLOUDSQL_CONNECTION_NAME, user = CLOUDSQL_USER,
            passwd = CLOUDSQL_PASSWORD, db = CLOUDSQL_DATABASE, connect_timeout = 1)
    except MySQLdb.OperationalError as e:
        db = None
    return db

def update_spaces(cursor, parkinglot, spot, status, gateway):
    """
    Updates the spot status from the Gateway

    Keyword arguments:
    cursor -- cursor object from the database
    parkinglot -- parking lot name of the spot
    spot -- spot number
    status -- status of the spot from the Gateway
    gateway -- Gateway number
    
    """
    cursor.execute('''UPDATE spaces SET Status=%s, Gateway=%s WHERE Parking_Lot=%s AND
                    Spots=%s''', [status, gateway, parkinglot, spot])
    if status == 'open':
        cursor.execute('SELECT User_ID FROM spaces WHERE Parking_Lot=%s AND Spots=%s',
                       [parkinglot, spot])
        if cursor.rowcount != 0:
            userid = cursor.fetchone()
            cursor.execute('UPDATE users SET Log_In_Status=0 WHERE id=%s', [userid[0]])
    cursor.execute('UPDATE spaces SET User_ID=NULL WHERE Parking_Lot=%s AND Spots=%s',
                   [parkinglot, spot])

def get_verified(cursor, gateway):
    """
    Returns the verified parking spots for the specific Gateway

    Keyword arguments:
    cursor -- cursor object from the database
    gateway -- Gateway number

    Returns:
    A dictionary containing a status key and list of verified parking spots. Returns status
    'none' if no spots have been verified.

    """
    dict_response = {}
    cursor.execute('''SELECT Parking_Lot, Spots, Status FROM spaces WHERE Complete=0 AND
                    Gateway=%s''', [gateway])
    if cursor.rowcount == 0:
        dict_response['status'] = 'none'
        return dict_response
    dict_response['status'] = 'ok'
    for line in cursor.fetchall():
        dict_response[line[0]+':'+str(line[1])] = line[2]
        cursor.execute('''UPDATE spaces SET Complete=1 WHERE Parking_Lot=%s AND Spots=%s AND
                        Gateway=%s''', [line[0], line[1], gateway])
    return dict_response

def verify_user(cursor, userid, bt_string):
    """
    Verifies the permit of the user and the spot type of the parking spot

    Keyword arguments:
    cursor -- cursor object from the database
    userid -- the user's ID
    bt_string -- the Bluetooth string from the spot

    Returns:
    A dictionary containing status 'verified' for a verifed parking spot, status 'payment'
    and account balance and daily rate of the spot, or status 'illegal' for illegal parking

    """
    dict_response = {}
    cursor.execute('SELECT Spot_Type, Status FROM spaces WHERE BT_String=%s', [bt_string])
    spot_and_status = cursor.fetchone()
    spot_permit = spot_and_status[0]
    spot_status = spot_and_status[1]
    cursor.execute('SELECT Permit_Type FROM users WHERE id=%s', [userid])
    user_permit = cursor.fetchone()
    cursor.execute('SELECT Parking_Lot, Spots FROM spaces WHERE BT_String=%s', [bt_string])
    entry = cursor.fetchone()
    parkinglot = entry[0]
    spot = entry[1]
    
    if (user_permit[0] in spot_permit) and (spot_status == 'busy'):
        dict_response['status'] = 'verified'
        cursor.execute('UPDATE spaces SET Status=%s WHERE BT_String=%s', ['closed', bt_string])
        cursor.execute('UPDATE spaces SET User_ID=%s WHERE BT_String=%s', [userid, bt_string])
        cursor.execute('UPDATE spaces SET Complete=%s WHERE BT_String=%s', [0, bt_string])
        cursor.execute('UPDATE users SET Log_In_Status=1 WHERE id=%s', [userid])

	fill_transaction(cursor, parkinglot, spot, userid, 0, 'Permit')
	fill_occupation(cursor, parkinglot, spot, 'closed', userid)
        return dict_response
    elif spot_status != 'busy':
        dict_response['status'] = 'Spot not busy'
        return dict_response
    elif 'Handicapped' in spot_permit:
        dict_response['status'] = 'illegal'
        cursor.execute('UPDATE spaces SET Complete=%s WHERE BT_String=%s', [0, bt_string])
        cursor.execute('UPDATE spaces SET Status=%s WHERE BT_String=%s', ['illegal', bt_string])
        cursor.execute('UPDATE users SET Log_In_Status=1 WHERE id=%s', [userid])
        cursor.execute('UPDATE spaces SET User_ID=%s WHERE BT_String=%s', [userid, bt_string])
        fill_occupation(cursor, parkinglot, spot, 'illegal', userid)
        return dict_response
    else:
        dict_response['status'] = 'payment'
        cursor.execute('SELECT Account_Balance FROM users WHERE id=%s', [userid])
        current_balance = cursor.fetchone()
        dict_response['current_balance'] = str(float(current_balance[0]))
        cursor.execute('SELECT Daily_Rate FROM spaces WHERE BT_String=%s', [bt_string])
        daily_rate = cursor.fetchone()
        dict_response['daily_rate'] = str(float(daily_rate[0]))
        return dict_response 

def complete_payment(cursor, userid, bt_string):
    """
    Confirms payment for the user

    Keyword arguments:
    cursor -- cursor object from the database
    userid -- the user's ID
    bt_string -- the spot's Bluetooth string

    Returns:
    A dictionary with status 'verified' for successful parking
    
    """
    dict_response = {}
    cursor.execute('SELECT Account_Balance FROM users WHERE id=%s', [userid])
    current_balance = cursor.fetchone()
    cursor.execute('SELECT Daily_Rate FROM spaces WHERE BT_String=%s', [bt_string])
    daily_rate = cursor.fetchone()
    cursor.execute('UPDATE users SET Account_Balance=%s WHERE id=%s',
                   [float(current_balance[0]) - float(daily_rate[0]), userid])
    dict_response['status'] = 'verified'
    dict_response['current_balance'] = str(float(current_balance[0]) - float(daily_rate[0]))
    cursor.execute('UPDATE spaces SET Status=%s WHERE BT_String=%s', ['closed', bt_string])
    cursor.execute('UPDATE spaces SET Complete=%s WHERE BT_String=%s', [0, bt_string])
    cursor.execute('UPDATE spaces SET User_ID=%s WHERE BT_String=%s', [userid, bt_string])
    cursor.execute('UPDATE users SET Log_In_Status=1 WHERE id=%s', [userid])
	
    cursor.execute('SELECT Parking_Lot, Spots FROM spaces WHERE BT_String=%s', [bt_string])
    entry = cursor.fetchone()
    parkinglot = entry[0]
    spot = entry[1]
    fill_transaction(cursor, parkinglot, spot, userid, daily_rate[0], 'Payment')
    fill_occupation(cursor, parkinglot, spot, 'closed', userid)
    return dict_response

def fill_occupation(cursor, parkinglot, spot, status, userid = None):
    cursor.execute("""INSERT INTO Occupation_History (Parking_Lot, Spots, Status,
                        Time_Stamp, id) VALUES (%s, %s, %s, %s, %s)""",[parkinglot, spot,
                        status, time.asctime(time.gmtime(time.time()-25200)),userid])

def fill_transaction(cursor, parkinglot, spot, userid, pay_amount, pay_type):
    cursor.execute("""INSERT INTO Transaction_History (User_ID, Time_Stamp, Payment_Ammount,
	                    Transaction_Type, Spaces, Parking_Lot) VALUES (%s, %s, %s, %s, %s, %s)""",
						[userid, time.asctime(time.gmtime(time.time()-25200)), pay_amount, pay_type,
						spot, parkinglot])
def get_spots(cursor, parkinglot):
    """
    Gets the number of open spots from the parking lot

    Keyword arguments:
    cursor -- cursor object from the database
    parkinglot -- the parking lot of the spot

    Returns:
    A dictionary with status 'ok' and 'spots' equal to the number of free spots
    
    """
    dict_response = {}
    cursor.execute("SELECT COUNT(*) FROM spaces WHERE Status='open' AND Parking_Lot=%s",
                   [parkinglot])
    if cursor.rowcount != 0:
        dict_response['status'] = 'ok'
        dict_response['spots'] = str(cursor.fetchone()[0])
        return dict_response
    dict_response['status'] = 'error'
    return dict_response

def get_mapinfo(cursor, parkinglot, spot):
    """
    Gets the spot info of the spot

    Keyword arguments:
    cursor -- cursor object from the database
    parkinglot -- the parking lot of the spot
    spot -- spot number

    Returns:
    A dictionary with status 'ok' for a successful message and fields of information for
    the spot, or status 'None' if the spot was not found
    
    """
    dict_response = {}
    cursor.execute('''SELECT Spot_Type, Status, User_ID, Daily_Rate, BT_String FROM
                    spaces WHERE Parking_Lot=%s AND Spots=%s''', [parkinglot, spot])
    if cursor.rowcount != 0:
        dict_response['status'] = 'ok'
        fetch = cursor.fetchone()
        dict_response['Spot_Type'] = str(fetch[0])
        dict_response['Status'] = str(fetch[1])
        userid = str(fetch[2])
        dict_response['User_ID'] = userid
        dict_response['Daily_Rate'] = str(fetch[3])
        dict_response['BT_String'] = str(fetch[4])
        cursor.execute('SELECT Permit_Type FROM users WHERE id=%s', [userid])
        if cursor.rowcount != 0:
            dict_response['Permit_Type'] = cursor.fetchone()[0]
            return dict_response
        else:
            dict_response['Permit_Type'] = 'None'
            return dict_response
    dict_response['status'] = 'error'
    return dict_response

def check_login(cursor, email, password):
    """
    Checks the login information from the mobile app with the database

    Keyword arguments:
    cursor -- cursor object from the database
    email -- email of the user
    password -- password of the user

    Returns:
    A dictionary of status 'ok' for a successful message and login 'verified' for a
    successful login, or login 'failed' if the fields do not match the database
    
    """
    dict_response = {}
    cursor.execute('SELECT COUNT(*) FROM users WHERE E_mail=%s AND Password=%s', [email, password])
    if cursor.rowcount != 0:
        response = str(cursor.fetchone()[0])
        if response == '1':
            dict_response['status'] = 'ok'
            dict_response['login'] = 'verified'
            return dict_response
        if response == '0':
            dict_response['status'] = 'ok'
            dict_response['login'] = 'failed'
            return dict_response
    dict_response['status'] = 'error'
    return dict_response

def get_mobileinfo(cursor, email, password):
    """
    Gets the account information of the user

    Keyword arguments:
    cursor -- cursor object from the database
    email -- email of the user
    password -- password of the user

    Returns:
    A dictionary of status 'ok' and fields of account information
    
    """
    dict_response = {}
    cursor.execute('SELECT Last_Name, First_Name, id, Permit_Type, E_mail, Account_Balance, Log_In_Status, License_Plate FROM users WHERE E_mail=%s AND Password=%s', [email, password])
    if cursor.rowcount != 0:
        dict_response['status'] = 'ok'
        fetch = cursor.fetchone()
        dict_response['Last_Name'] = str(fetch[0])
        dict_response['First_Name'] = str(fetch[1])
        userid = str(fetch[2])
        dict_response['id'] = userid
        dict_response['Permit_Type'] = str(fetch[3])
        dict_response['E_mail'] = str(fetch[4])
        dict_response['Account_Balance'] = str(fetch[5])
        dict_response['Log_In_Status'] = str(fetch[6])
        dict_response['License_Plate'] = str(fetch[7])
        cursor.execute('SELECT Parking_Lot, Spots FROM spaces WHERE User_ID=%s', [userid])
        fetch2 = cursor.fetchone()
        if cursor.rowcount != 0:
            dict_response['Parking_Lot'] = str(fetch2[0])
            dict_response['Spots'] = str(fetch2[1])
            return dict_response
        else:
            dict_response['Parking_Lot'] = 'None'
            dict_response['Spots'] = 'None'
            return dict_response
    dict_response['status'] = 'error'
    return dict_response

def get_parking_status(cursor, parkinglot):
    """
    Gets the spots and their status

    Keyword arguments:
    cursor -- cursor object from the database
    parkinglot -- the specified parking lot

    Returns:
    A dictionary of spots and their statuses, with status 'ok' indicating a successful
    message
    
    """
    dict_response = {}
    cursor.execute('SELECT Spots, Status FROM spaces WHERE Parking_Lot=%s', [parkinglot])
    if cursor.rowcount != 0:
        dict_response['status'] = 'ok'
        for row in cursor:
            dict_response[str(row[0])] = str(row[1])
        return dict_response
    dict_response['status'] = 'error'
    return dict_response

def get_btid(cursor, data):
    """
    Gets the parking lots and spots of the list of Bluetooth strings

    Keyword arguments:
    cursor -- cursor object from the database
    data -- dictionary of Bluetooth strings

    Returns:
    A dictionary of Bluetooth keys and their corresponding parking lot and spot number
    
    """
    dict_response = {}
    dict_response['status'] = 'ok'
    for line in data:
        cursor.execute('SELECT Parking_Lot, Spots FROM spaces WHERE BT_String=%s', [data[line]])
        if cursor.rowcount != 0:
            response = cursor.fetchone()
            spotid = str(str(response[0])+':'+str(response[1]))
            dict_response[line] = spotid
        else:
            dict_response['status'] = 'error'
    return dict_response

##### WSGI handlers for GET and POST requests #####
# Website handlers
class MainPage(webapp2.RequestHandler):
    def get(self):
        path = os.path.join(os.path.dirname(__file__), 'index.html')
        self.response.out.write(template.render(path, {}))

# Handles user search and listing account information
class Users(webapp2.RequestHandler):

    # Returns search form and list of accounts
    def get(self):
        path = os.path.join(os.path.dirname(__file__), 'Users.html')
        self.response.out.write(template.render(path, {}))

    # Searches for user accounts using form fields and returns the accounts
    def post(self):
        first = self.request.get('First_Name')
        if first == '':
            first = '%%'
        last = self.request.get('Last_Name')
        if last == '':
            last = '%%'
        userid = self.request.get('id')
        if userid == '':
            userid = '%%'
        permit_type = self.request.get('Permit_Type')
        if permit_type == '':
            permit_type = '%%'
        license_plate = self.request.get('License_Plate')
        if license_plate == '':
            license_plate = '%%'

        db = connect_to_cloudsql()
        cursor = db.cursor()
        cursor.execute('''SELECT Last_Name, First_Name, id, Permit_Type, E_mail, Account_Balance,
                       Password, Log_In_Status, License_Plate FROM users WHERE First_Name LIKE %s AND
                       Last_Name LIKE %s AND id LIKE %s AND Permit_Type LIKE %s AND License_Plate LIKE %s''',
                       [first, last, userid, permit_type, license_plate])
        account_list = list()
        for row in cursor:
            account = {}
            account['last'] = str(row[0])
            account['first'] = str(row[1])
            account['userid'] = str(row[2])
            account['permit'] = str(row[3])
            account['email'] = str(row[4])
            account['acc_balance'] = str(row[5])
            account['password'] = str(row[6])
            account['log_status'] = str(row[7])
            account['lic_plate'] = str(row[8])
            account_list.append(account)
        
        template_values = {
            'account_list': account_list,
        }
        
        template = JINJA_ENVIRONMENT.get_template('Users.html')
        self.response.write(template.render(template_values))

        cursor.close()
        db.commit()
        db.close()
        
# Handler to edit user account information
class Edit(webapp2.RequestHandler):

    # Returns a form for editing user info
    def get(self):
        account = {}
        account['last'] = self.request.get('last')
        account['first'] = self.request.get('first')
        account['userid'] = self.request.get('userid')
        account['permit'] = self.request.get('permit')
        account['email'] = self.request.get('email')
        account['acc_balance'] = self.request.get('acc_balance')
        account['lic_plate'] = self.request.get('lic_plate')

        template_values = {
            'account': account,
        }
        
        template = JINJA_ENVIRONMENT.get_template('Edit.html')
        self.response.write(template.render(template_values))

    # Submits form and updates the user info in the database
    def post(self):
        first = self.request.get('First_Name')
        if first == '':
            first = 'NULL'
        last = self.request.get('Last_Name')
        if last == '':
            last = 'NULL'
        userid = self.request.get('id')
        if userid == '':
            userid = '0'
        permit_type = self.request.get('Permit_Type')
        if permit_type == '':
            permit_type = 'NULL'
        email = self.request.get('email')
        if email == '':
            email = 'NULL'
        account_balance = self.request.get('account_balance')
        if account_balance == '':
            account_balance = '0'
        license_plate = self.request.get('License_Plate')
        if license_plate == '':
            license_plate = 'NULL'

        db = connect_to_cloudsql()
        cursor = db.cursor()
        cursor.execute('''UPDATE users SET Last_Name=%s, First_Name=%s, id=%s, Permit_Type=%s,
                            E_mail=%s, Account_Balance=%s, License_Plate=%s WHERE id=%s''',
                           [last, first, userid, permit_type, email, account_balance, license_plate, userid])

        template = JINJA_ENVIRONMENT.get_template('Changes.html')
        self.response.write(template.render())

        cursor.close()
        db.commit()
        db.close()

# Handler that deletes the selected account from the search list
class Delete(webapp2.RequestHandler):
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        data = json.loads(self.request.body)
        db = connect_to_cloudsql()
        cursor = db.cursor()

        cursor.execute('DELETE FROM users WHERE id=%s', [data['userid']])

        cursor.close()
        db.commit()
        db.close()
        
# Handler for adding new users to the database
class Add(webapp2.RequestHandler):

    # Submits and creates the new user
    def post(self):
        first = self.request.get('First_Name')
        if first == '':
            first = 'NULL'
        last = self.request.get('Last_Name')
        if last == '':
            last = 'NULL'
        userid = self.request.get('id')
        if userid == '':
            userid = '0'
        permit_type = self.request.get('Permit_Type')
        if permit_type == '':
            permit_type = 'NULL'
        email = self.request.get('email')
        if email == '':
            email = 'NULL'
        account_balance = self.request.get('account_balance')
        if account_balance == '':
            account_balance = '0'
        password = self.request.get('password')
        if password == '':
            password = 'NULL'
        license_plate = self.request.get('License_Plate')
        if license_plate == '':
            license_plate = 'NULL'

        db = connect_to_cloudsql()
        cursor = db.cursor()
        cursor.execute("""INSERT INTO users VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)""",
                       [last, first, userid, permit_type, email, account_balance, password, "0",
                        license_plate])
        
        template = JINJA_ENVIRONMENT.get_template('Changes.html')
        self.response.out.write(template.render())
        
        cursor.close()
        db.commit()
        db.close()

# Handles transaction/occupation searches 
class Logs(webapp2.RequestHandler):
    def get(self):
        path = os.path.join(os.path.dirname(__file__), 'Logs.html')
        self.response.out.write(template.render(path, {}))

    # Lists searches in the Transaction table
    def post(self):
        date = self.request.get('date')
        if date == '':
            date = '%%'
        else:
            date = date.split()
            if len(date[1]) == 1:
                date = '%' + date[0] + '  ' + date[1] + '%'
            else:
                date = '%' + date[0] + ' ' + date[1] + '%'
        parking_lot = self.request.get('parking_lot')
        if parking_lot == '':
            parking_lot = '%%'
        spot = self.request.get('spot')
        if spot == '':
            spot = '%%'
        payment_type = self.request.get('payment_type')
        if payment_type == '':
            payment_type = '%%'
        userid = self.request.get('userid')
        if userid == '':
            userid = '%%'

        db = connect_to_cloudsql()
        cursor = db.cursor()
        cursor.execute('''SELECT * FROM Transaction_History WHERE User_ID LIKE %s AND Time_Stamp LIKE %s
                        AND Transaction_Type LIKE %s AND Spaces LIKE %s AND Parking_Lot LIKE %s''',
                       [userid, date, payment_type, spot, parking_lot])
        transaction_list = list()
        
        for row in cursor:
            transaction = {}
            transaction['userid'] = str(row[0])
            transaction['timestamp'] = str(row[1])
            transaction['paymentamount'] = str(row[2])
            transaction['transactiontype'] = str(row[3])
            transaction['spot'] = str(row[4])
            transaction['parkinglot'] = str(row[5])
            transaction_list.append(transaction)
        
        template_values = {
            'transaction_list': transaction_list,
        }
        
        template = JINJA_ENVIRONMENT.get_template('Logs.html')
        self.response.write(template.render(template_values))

        cursor.close()
        db.close()

# Lists searches in the occupation table
class Occupation(webapp2.RequestHandler):
    def post(self):
        date = self.request.get('date')
        if date == '':
            date = '%%'
        else:
            date = date.split()
            if len(date[1]) == 1:
                date = '%' + date[0] + '  ' + date[1] + '%'
            else:
                date = '%' + date[0] + ' ' + date[1] + '%'
        parking_lot = self.request.get('parking_lot')
        if parking_lot == '':
            parking_lot = '%%'
        spot = self.request.get('spot')
        if spot == '':
            spot = '%%'
        status = self.request.get('status')
        if status == '':
            status = '%%'
        userid = self.request.get('userid')
        if userid == '':
            userid = '%%'

        db = connect_to_cloudsql()
        cursor = db.cursor()
        cursor.execute('''SELECT * FROM Occupation_History WHERE (id LIKE %s OR id IS NULL)
                        AND Time_Stamp LIKE %s AND Status LIKE %s AND Spots LIKE %s AND
                        Parking_Lot LIKE %s''', [userid, date, status, spot, parking_lot])
        occupation_list = list()
        
        for row in cursor:
            occupation = {}
            occupation['spot'] = str(row[0])
            occupation['status'] = str(row[1])
            occupation['timestamp'] = str(row[2])
            occupation['parkinglot'] = str(row[3])
            occupation['id'] = str(row[4])
            occupation_list.append(occupation)
        
        template_values = {
            'occupation_list': occupation_list,
        }
        
        template = JINJA_ENVIRONMENT.get_template('Logs.html')
        self.response.write(template.render(template_values))

        cursor.close()
        db.close()
        
# Returns HTML file
class NorthRemote(webapp2.RequestHandler):
    def get(self):
        path = os.path.join(os.path.dirname(__file__), 'NorthRemote.html')
        self.response.out.write(template.render(path, {}))

class EastRemote(webapp2.RequestHandler):
    def get(self):
        path = os.path.join(os.path.dirname(__file__), 'EastRemote.html')
        self.response.out.write(template.render(path, {}))

class Baskin(webapp2.RequestHandler):
    def get(self):
        path = os.path.join(os.path.dirname(__file__), 'Baskin.html')
        self.response.out.write(template.render(path, {}))

# Gets the specified spot info for the parking map
class get_spotinfo(webapp2.RequestHandler):
    def get(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        db = connect_to_cloudsql()
        cursor = db.cursor()
        dict_response = get_mapinfo(cursor, self.request.get('Parking_Lot'), self.request.get('Spots'))
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.close()

# Gets the list of spots and their statuses for the parking map
class get_parkinglot(webapp2.RequestHandler):
    def get(self):
        dict_response = {}
        self.request.headers['Content-Type'] = 'text/plain'
        db = connect_to_cloudsql()
        cursor = db.cursor()
        dict_response = get_parking_status(cursor, self.request.get('Parking_Lot'))
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.close()
        
##### System handlers for parking verification #####
# Gateway handlers for updating sensor status and sending LED status
class gateway(webapp2.RequestHandler):
    
    # Return verified spots to Gateway
    def get(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Update'):
            dict_response['status'] = 'Not a Update message'
            self.response.write(json.dumps(dict_response))
            return
        gateway = self.request.headers['Gateway']
        db = connect_to_cloudsql()
        cursor = db.cursor()
        dict_response = get_verified(cursor, gateway)
        self.response.write(json.dumps(dict_response))
        
        db.commit()
        cursor.close()
        db.close()

    # Receive sensor updates and load to database
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Update'):
            dict_response['status'] = 'error'
            self.response.write(json.dumps(dict_response))
            return
        db = connect_to_cloudsql()
        cursor = db.cursor()
        data = json.loads(self.request.body)
        gateway = self.request.headers['Gateway']
        for line in data:
            temp = line.split(':')
            update_spaces(cursor, temp[0], temp[1], data[line], gateway)
            fill_occupation(cursor, temp[0], temp[1], data[line])
        dict_response['status'] = 'ok'
        self.response.write(json.dumps(dict_response))
        
        cursor.close()
        db.commit()
        db.close()

##### Mobile app handlers for user verifcation #####
class mobileapp(webapp2.RequestHandler):
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Verify'):
            dict_response['status'] = 'Not a Verify message'
            self.response.write(json.dumps(dict_response))
            return
        db = connect_to_cloudsql()
        cursor = db.cursor()
        data = json.loads(self.request.body)
        dict_response = verify_user(cursor, data['userid'], data['bt_string'])
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.commit()
        db.close()

# Payment handler for user payment step
class payment(webapp2.RequestHandler):
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Payment'):
            dict_response['status'] = 'Not a Payment message'
            self.response.write(json.dumps(dict_response))
            return
        db = connect_to_cloudsql()
        cursor = db.cursor()
        data = json.loads(self.request.body)
        dict_response = complete_payment(cursor, data['userid'], data['bt_string'])
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.commit()
        db.close()

# Retrieves number of open spots from a specified parking lot
class spots(webapp2.RequestHandler):
    def get(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Spots'):
            dict_response['status'] = 'Not a Spots message'
            self.response.write(json.dumps(dict_response))
            return
        db = connect_to_cloudsql()
        cursor = db.cursor()
        dict_response = get_spots(cursor, self.request.get('Parking_Lot', default_value = 'error'))
        self.response.write(json.dumps(dict_response))
        
        cursor.close()
        db.close()

# Handler for verifying user login
class login(webapp2.RequestHandler):
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Login'):
            dict_response['status'] = 'Not a Login message'
            self.response.write(json.dumps(dict_response))
            return
        db = connect_to_cloudsql()
        cursor = db.cursor()
        data = json.loads(self.request.body)

        dict_response = check_login(cursor, data['email'], data['password'])
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.close()

# Retrieves account information for the mobile app
class mobileinfo(webapp2.RequestHandler):
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Info'):
            dict_response['status'] = 'Not a Info message'
            self.response.write(json.dumps(dict_response))
            return
        data = json.loads(self.request.body)
        db = connect_to_cloudsql()
        cursor = db.cursor()

        dict_response = get_mobileinfo(cursor, data['email'], data['password'])
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.close()

# Receives Bluetooth strings from mobile app and returns their parking lot and spot
class bluetooth(webapp2.RequestHandler):
    def post(self):
        dict_response = {}
        self.response.headers['Content-Type'] = 'text/plain'
        if (self.request.headers['Message_Type'] != 'Bluetooth'):
            dict_response['status'] = 'Not a Bluetooth message'
            self.response.write(json.dumps(dict_response))
            return
        data = json.loads(self.request.body)
        db = connect_to_cloudsql()
        cursor = db.cursor()
        dict_response = get_btid(cursor, data)
        self.response.write(json.dumps(dict_response))

        cursor.close()
        db.close()

application = webapp2.WSGIApplication([
    ('/gateway', gateway),
    ('/mobileapp', mobileapp),
    ('/payment', payment),
    ('/spots', spots),
    ('/get_spotinfo', get_spotinfo),
    ('/login', login),
    ('/mobileinfo', mobileinfo),
    ('/Users', Users),
    ('/NorthRemote', NorthRemote),
    ('/EastRemote', EastRemote),
    ('/Baskin', Baskin),
    ('/get_parkinglot', get_parkinglot),
    ('/Add', Add),
    ('/Logs', Logs),
    ('/Occupation', Occupation),
    ('/Edit', Edit),
    ('/bluetooth', bluetooth),
    ('/Delete', Delete),
    ('/', MainPage), ], debug=True)
