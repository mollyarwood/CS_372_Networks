README
Title: Project 1
Date: 05/05/2017
Author: Molly Arwood
Class: CS_372_Intro_to_Networks

-------------------------
Included in zip file:
-------------------------
	chatserve (a python file)
	chatclient.c
	makefile
	README

-------------------------
chatclient.c Compilation
-------------------------
Enter "make" in the command line


-------------------------
Command Line Usage
-------------------------
chatserve usage:
	./chatserve <port number>

chatclient usage:
	./chatclient <server hostname> <port number>



*chatserve must be started before chatclient
**port numbers must match

------------------------
Description of source code
------------------------
chatserve: 
	Sets up a port to listen for connections. Waits for
	client to attempt to connect. Once client connection
	is accepted, server waits for client to make first
	contact. Messages can be sent to and received from
	client. If client disconnects, server waits for 
	another connection from a client. 

chatclient:
	Sets up a port in order to connect with a server. 
	Prompts user for input chat message. Sends message
	to server. If server disconnects or if user
	disconnects, program will end. 
