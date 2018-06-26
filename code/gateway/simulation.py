

import pexpect
import random
import sys
import os
import time

userList =[]
numOfUsers = 0


readfile = open("sim.txt","r")
for line in readfile:
	line = line.strip("\r\n")
	line = line.split(" ")
	userList.append([line[0],line[1]])
	numOfUsers +=1

#print userList
svar = raw_input("Max Number of activeUsers? ")

iVar = int(svar)
if (iVar >50) or (iVar < 1):
	iVar = 10
activeUsers = random.randint(1,iVar)

activeUsers = iVar

print "We chose ",activeUsers,"activeUsers"

numOfGateways = activeUsers/10
if activeUsers % 10 > 0:
	numOfGateways+=1

print numOfGateways


lotNum=0
userNum = 0
parkingLots=['EastRemote','Baskin','NorthRemote']
gatewayNum = 1;
ogPort = 1111;
gateway= [0] *numOfGateways
sensor = [0] *activeUsers
app = [0] * activeUsers


port = ogPort
#spawn 'numOfGateways' ammount of gateway processes
# z is 0 - numOfGateways -1
for z in range(numOfGateways):
	gateway[z] = pexpect.spawn('./server '+str(port)+" "+
	             parkingLots[lotNum]+" "+str(gatewayNum)
	             , timeout=5) #dispay prompt within 5 sec
	
	try:
		c = gateway[z].expect("Waiting for clients...\r\n")
		
		print "C is ",str(c)
		if c == 0: 
			print("Got gateway prompt")
		if c == 1:
			print("Error: timeout")
			gateway[z].kill(0)
	except:
		print("Error, exiting")
		sys.exit()
	#val = gateway[z].read()
	#print val
	port+=1
	gatewayNum+=1

#spawn 10 sensors for each gateway
port = ogPort
 
users = 0
for z in range(numOfGateways):
	spot = 1
	for i in range(10):
		sensor[userNum] = pexpect.spawn('./client '+str(port)+' 1'+
							parkingLots[lotNum]+":"+str(spot)+
							" busy", timeout = 5)
		#val = sensor[userNum].read()
		#print val
		try:
			c = sensor[userNum].expect('Waiting to recv LED status...\r\n')
			if c == 0: 
				print("Got sensor prompt")
				userNum+=1
				users +=1
				spot+=1
			if c == 1:
				print("Error: timeout")
				gateway[z].kill(0)
		except:
			print("Error, exiting")
			sys.exit()

		if users == activeUsers:
			break


#spawn activeUsers number of app proccesses
var = raw_input("Probaibilty of successful users (0 - 1): ")
successfulProbability = float(var)

if successfulProbability > 1.0 or successfulProbability < 0:
	successfulProbability = 1.0
failProbability = 1.0 - successfulProbability
appNum = 0

#send successful messages
if successfulProbability > 0:
	#calculate number of successful users to send
	successUsersFloat = successfulProbability * activeUsers
	#Add 
	if successUsersFloat % 1 != 0:
		successUsers+=1
	successUsers = int(successUsersFloat)
	failUsers = activeUsers - successUsers
	print "Succesful users is ", successUsers
	print "failUsers is ", failUsers
	if successUsers+failUsers == activeUsers:
		print "Calculated successful and fialed users successfully"
	else:
		print "Did not get it right"
	for i in range(successUsers):
		print "About to spwan: python mobileapp.py ", userList[appNum][0], " ", userList[appNum][1]," "+"time"
		app[appNum] = pexpect.spawn('python mobileapp.py '+
						userList[appNum][0]+" "+
						userList[appNum][1]+" "+
						'0')
		
		
		#time.sleep(1)
		val = app[appNum].read()
		print val
		appNum +=1
		if appNum == successUsers:
			print "Just Like the Simulations"
			break
print "Everyone arrived, going to sleep"
exit
time.sleep(random.randint(1,5))
print "Some users are leaving"
leavingUsers = random.randint(1,successUsers)
userNum = 0
lotNum = 0
spot = 1
for i in range(leavingUsers):
	print "About to spawn ./client "	
	sensor[userNum] = pexpect.spawn('./client '+str(port)+' 1'+
							parkingLots[lotNum]+":"+str(spot)+
							" open", timeout = 5)
	#val = sensor[userNum].read()
	#print val
	try:
		c = sensor[userNum].expect('Waiting to recv LED status...\r\n')
		if c == 0: 
			print("Got sensor prompt")
			userNum+=1
			users +=1
			spot+=1
		if c == 1:
			print("Error: timeout")
			gateway[z].kill(0)
	except:
		print("Error, exiting")
		sys.exit()

	if users == activeUsers:
		break

print "Going to sleep again, userNum is ", userNum
time.sleep(4)
#userNum-=1
#users -=1
#spot-=
print "opening all spots, work day is done"
remainingUsers = successUsers - leavingUsers
#remainingUsers+=1
if(remainingUsers != 0):
	print "leaving Users: ",leavingUsers,"remaining Users: ",remainingUsers
	for i in range(remainingUsers):
		print "About to spawn ./client UserNum is ",userNum
		sensor[userNum] = pexpect.spawn('./client '+str(port)+' 1'+
								parkingLots[lotNum]+":"+str(spot)+
								" open", timeout = 5)
		#val = sensor[userNum].read()
		#print val

		try:
			c = sensor[userNum].expect('Waiting to recv LED status...\r\n')
			if c == 0: 
				print("User left")
				userNum+=1
				users +=1
				spot+=1
			if c == 1:
				print("Error: timeout")
				gateway[z].kill(0)
		except:
			print("Error, exiting")
			sys.exit()

		if users == activeUsers:
			break
time.sleep(2)	
#send "failed" messages
#if failProbability > 0:
