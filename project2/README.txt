README
Title: Project 2
Date: 06/05/2017
Author: Molly Arwood
Class: CS_372_Intro_to_Networks

-------------------------
Included in zip file:
-------------------------
	ftclient (a python file)
	ftserver.c
	README

-------------------------
ftserver.c Compilation
-------------------------
Enter "gcc ftserver.c -o ftserver" into the command line


-------------------------
Command Line Usage
-------------------------
ftserver usage:
	ftserver <port number> &

ftclient usage:
	To receive list of Files in Server Directory:
		ftclient.py <server hostname> <server port> <-l> <data port>

	To Receive the Contents of a Specified File:
		ftclient.py <server hostname> <server port> <-g> <filename> <data port>


*ftserver must be started before ftclient
**server port numbers must match

------------------------
Description of source code
------------------------
ftserver: 
	Sets up a port to listen for connections. Waits for
	client to attempt to connect. Once client connection
	is accepted, server waits for client to send commands.
	ftserver interprets commands and opens up a new TCP
	connection with client. ftserver sends back one of
	three things on this new connection:
		1. List of the Files in the Server's Current Directory
		2. Contents of a File in the Server's Current Diectory
		3. Error Message 

ftclient:
	Sets up a port in order to connect with a server. 
	Interprets command line entry and sends message
	to server. ftclient sets up a listening port and waits for
	the server to contact it on that port. Once contacted,
	ftclient will interpret the message and close the socket. 
