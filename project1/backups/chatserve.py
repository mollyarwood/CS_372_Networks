#!/usr/bin/env python3
#***************************************************************
#Name: Molly Arwood
#Term: Spring 2017
#Class: CS_372_Networks
#chatserve.py
#***************************************************************

from socket import *
import sys
import os


#def chat(handle):
	#write chat code here



if __name__ == '__main__':

	#check cmd line args
	if len(sys.argv) < 1:
		print("USAGE: chatserve.py <port#> \n")

	#define handle
	handle = "you"
#	handle = input("Enter a username of 10 chars or less: ")


	#*****************************************
	#set up socket to listen for connections
	#*****************************************
	#serverName = gethostbyname(sys.argv[1])
	serverPort = int(sys.argv[1])
	#TCP connection
	serverSocket = socket(AF_INET,SOCK_STREAM)
	#bind port to socket number chosen
	try:
		serverSocket.bind(('',serverPort)) 
	except socket.error as msg:
		print("Socket binding error \n")
	#listen for connections
	serverSocket.listen(1)
	print("The server is ready to receive \n")



	while 1:
		#create new socket upon receiving request
		connectionSocket, addr = serverSocket.accept()
		sentence = connectionSocket.recv(511)
		print("server received: {}".format(sentence))
		connectionSocket.close()

	serverSocket.close()
	print("serverSocket closed. \n")	

