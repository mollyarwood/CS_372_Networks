#!/usr/bin/env python3
#***************************************************************
 #Name: Molly Arwood
 #Term: Spring 2017
 #Class: CS_372_Networks
 #ftclient.py
#**************************************************************

from socket import *
import sys
import os
import signal
import platform

#************************************************
 #Function: sig_handler
 #Definition:
 #Parameters:
 #Pre-Conditions: 
 #Post-Conditions:
 #Author/Source: 
#************************************************
def sig_handler(signal, frame):
	print("\nSignal Caught. Now Exiting")
	sys.exit(0)


#************************************************
 #Function: initConnect
 #Definition: connects to the server socket
 #Parameters: portnumber and server hostname
 #Pre-Conditions: gather portnumber and hostname
	#from cmd line
 #Post-Conditions:connection established with
	#server
 #Author/Source:Molly, CS_372_Network Lecture 15,
	#slide X 
#************************************************
def initConnect(portNum, serverHost):
	print("initConnect")

	serverName = serverHost
	serverPort = portNum
	clientSocket = socket(AF_INET, SOCK_STREAM)
	clientSocket.connect((serverName, serverPort))

	return clientSocket




#************************************************
 #Function: ServerSeupt
 #Definition: sets up the data socket to listen 
	#for the server 
 #Parameters: int dataport
 #Pre-Conditions: conntrol socket setup
 #Post-Conditions: data socket setup
 #Author/souorce: CS_372 Lecture 15
#&=**********************************************
def serverSetup(dataPort):
	print("serverSetup")
	serverSocket = socket(AF_INET, SOCK_STREAM)
	#bind port to socket number chosen
	try:
		serverSocket.bind(('', dataPort))
	except socket.error as msg:
		print("Server: Socket binding error \n")
	#listen for connection
	serverSocket.listen(1)
	print("The server is ready to receive connection \n")
	return serverSocket



#************************************************
 #Function: CheckUsage
 #Definition: checks the command line args
 #Parameters: none
 #Pre-Conditions: user enters command into
	#command line
 #Post-Conditions: args declared valid or invalid
 #Author/souorce: Molly 
#&=**********************************************
def checkUsage():
	print("argv0 {}".format(sys.argv[0]))
	if len(sys.argv) < 4 or len(sys.argv) > 6:
		print("ERROR: incorrect number of arguments\n")
		print("USAGE: ftclient <server_host> <server_port> <-g> <filename> <data_port> OR\n")
		print("ftclient <server_host> <server_port> <-l> <data_port> \n")
		sys.exit(1)
	elif sys.argv[3] != "-g" and sys.argv[3] != "-l":
		print("ERROR: invalid command {}\n".format(sys.argv[3]))
		sys.exit(1)

#************************************************
 #Function: makeRequest
 #Description: sends request to server on control
	#port
 #Parameters: command, port number
 #Pre-Condistions:
 #Post-Conditions:
 #Author/Source: Molly
#************************************************
def makeRequest(command, dataPort, filename, socketFD):

	if command == "-g":
		toSend = command + "|" + str(dataPort) + "|" + filename
	elif command == "-l":
		toSend = command + "|" + str(dataPort)

	socketFD.send(bytes(toSend, 'UTF-8'))


#************************************************
 #Function: receiveFile
 #Description: parses and outputs received msgs
 #Parameters: msg from server
 #Pre-Conditions: server sends message to client
 #Post-Conditions: msg is displayed
 #Author/Source: Molly
#************************************************
def receiveMsg(command, dataSocket, filename):

	if command == "-l":
		print("Directory Files:\n\n")
		serverMsg = dataSocket.recv(2000)
		newMsg = serverMsg.decode("utf-8")
		parsed = newMsg.split("|")
		for i in parsed:
			print("{}\n".format(i))
	if command == "-g":
		print("Receiving File From Server\n")
		filename2 = filename + '2'
		fp = open(filename2, "w")
		serverMsg = dataSocket.recv(500)
		newMsg = serverMsg.decode("utf-8")
		while not "%!?%" in newMsg:
			fp.write(newMsg)
			fp.write("\n")

		fp.close()
		print("Transfer Complete. New File: {}\n".format(filename2))


#*************************************************
 #Main
#************************************************
if __name__ == '__main__':
	
	#Check cmd line args
	checkUsage()
 
	#grab cmd line args
	serverHost = sys.argv[1] + ".engr.oregonstate.edu"
	portNum = int(sys.argv[2])
	command = sys.argv[3]
	if command == "-g":
		filename = sys.argv[4]
		dataPort = int(sys.argv[5])
	else:
		filename = '\0'
		dataPort = int(sys.argv[4])

	#Set up the server address struct and connect
	socketFD = initConnect(portNum, serverHost);

	#send command ('-1' or -g<filename>)
	makeRequest(command, dataPort, filename, socketFD);
	
	#set up socket to listen for connection.
	dataSocket = serverSetup(dataPort)
	ready = "ready"
	socketFD.send(bytes(ready, 'UTF-8'))

	while 1:
		#create new socket upon receiving request
		connectionSocket, addr = dataSocket.accept()
		print("data connection open\n")
		receiveMsg(command, dataSocket, filename);
		print("Closing Connection\n")
		dataSocket.close()




