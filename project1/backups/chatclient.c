/***************************************************************
 * Name: Molly Arwood
 * Term: Spring 2017
 * Class: CS_372_Networks
 * chatclient.c
 * ************************************************************/


#include <stdio.h>   //for fgets, printf, etc.
#include <stdlib.h>
#include <unistd.h>  //for pid stuff
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //for sockaddr_in
#include <netdb.h>      //get hostbyname(), etc.
#include <sys/ioctl.h>  

void main(int argc, char *argv[]) {

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char clientName[11];
	char sendBuffer[700];
	char clientMsg[512];
	char recBuffer[512];


	if (argc < 2) {
		printf("USAGE: %s server hostname port \n", argv[0]);
		exit(1);
	}
	
	printf("port number: %s \n", argv[2]);

	/*********************************************
	 //set up the server address struct
	**********************************************/
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear out server address
	portNumber = atoi(argv[2]);  //get port number and convert to int
	serverAddress.sin_family = AF_INET;  //create network-capable socket (IPv4)
	serverAddress.sin_port = htons(portNumber);   //store port number in Network Byte Order
	serverHostInfo = gethostbyname(argv[1]);  //convert the machine name into a special form of address - localhost
	if (serverHostInfo == NULL) {
		printf("CLIENT: ERRPR, no such host\n");
		exit(0);
	} 
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);  //copy in address


	/*********************************************
	//set up the socket
	*********************************************/
	socketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the TCP socket
	if (socketFD < 0){ 
		printf("CLIENT: ERROR opening socket\n");
		exit(1);
	}

	/*********************************************
 	// Connect to server
 	*********************************************/
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		printf("CLIENT: ERROR connecting to port\n");
		exit(2);
	}
	
	/*********************************************
	//get name of client & provide instructions
	*********************************************/
	memset(clientName, '\0', sizeof(clientName)); //Clear out the array
	printf("Enter user handle: ");
	fgets(clientName, sizeof(clientName), stdin);
	clientName[strcspn(clientName, "\n")] = '\0'; //Remove the trailing \n, if any
	printf("wait until handle appears to enter message\n");
	printf("Enter 'Quit' to end program\n");

	/*********************************************
 	//Send/Receive messages to server
 	*********************************************/
	while (1) {
		//prepare input fields
		memset(sendBuffer, '\0', sizeof(sendBuffer));  //clear out buffer each time
		memset(clientMsg, '\0', sizeof(clientMsg));  //clear out message




		strcat(sendBuffer, clientName);
		strcat(sendBuffer, "> ");
		strcat(sendBuffer, clientMsg);




		//send message to server
		charsWritten = send(socketFD, sendBuffer, strlen(sendBuffer), 0); //write to server
		if (charsWritten < 0) {
			printf("CLIENT: ERROR reading from socket");
			exit(1);
		}
		if (charsWritten < strlen(sendBuffer))
			printf("CLIENT: WARNING: Not all data written to socket!\n");



		//receive message from server
		memset(recBuffer, '\0', sizeof(recBuffer));
		charsRead = recv(socketFD, recBuffer, sizeof(recBuffer), 0);
		if (charsRead < 0) {
			printf("CLIENT: ERROR reading from socket\n");
			exit(1);
		}
		if (strcmp(recBuffer, "-1") == 0) {
			close(socketFD);
			exit(1);
		}
		printf("%s \n", recBuffer);

	}


	close(socketFD);

}





