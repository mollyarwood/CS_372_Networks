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


//Prototypes
void chat(int socketFD);
int Initconnect(char *, char*); 
void receiveMsg(int socketFD);


void main(int argc, char *argv[]) {

	int socketFD, charsRead;
	char* portNumber;
	char* serverHost;
//	char recBuffer[512];


	//check arguments
	if (argc < 2) {
		printf("USAGE: %s server hostname port \n", argv[0]);
		exit(1);
	}
	
	printf("port number: %s \n", argv[2]);
	portNumber = argv[2];
	serverHost = argv[1];


	//setup connection and start chat
	socketFD = Initconnect(portNumber, serverHost);
	chat(socketFD);
	
	close(socketFD);
}



/***************************************************************
 * Function: Initconnect
 * Description: connects to the serrver socket
 * Parameters: portnumber and server hostname
 * Pre-Conditions: gather portnumber and hostname from cmd line
 * Post-Conditions: connection established with server
 * Author/Source: Ben Brewster (OSU prof)
 * ************************************************************/
int Initconnect(char* portNbr, char* serverHost) {

	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	/*********************************************
	 //set up the server address struct
	**********************************************/
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear out server address
	portNumber = atoi(portNbr);  //get port number and convert to int
	serverAddress.sin_family = AF_INET;  //create network-capable socket (IPv4)
	serverAddress.sin_port = htons(portNumber);   //store port number in Network Byte Order
	serverHostInfo = gethostbyname(serverHost);  //convert the machine name into a special form of address - localhost
	if (serverHostInfo == NULL) {
		printf("CLIENT: ERROR, no such host\n");
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
	
	return socketFD;
}



/***************************************************************
 * Function: chat
 * Description: establishes handle, gathers user msg, sends msg
 * 	to server. Calls receiveMsg() to get server msg back.
 * Parameters: socket file descriptor
 * Pre-Conditions:server connection established
 * Post-Condistions: client communication with server occured
 * Author/Source: Ben Brewster (OSU Prof) and Molly A
 * ************************************************************/
void chat(int socketFD) {

	char sendBuffer[700];
	char clientName[11];
	char clientMsg[512];
	int charsWritten;

	/*********************************************
	//get name of client & provide instructions
	*********************************************/
	memset(clientName, '\0', sizeof(clientName)); //Clear out the array
	printf("Enter user handle of 10 characters or less: ");
	fgets(clientName, sizeof(clientName), stdin);
	clientName[strcspn(clientName, "\n")] = '\0'; //Remove the trailing \n, if any
	printf("\n\nWait until handle appears to enter message\n");
	printf("Enter '\\quit' to end program\n\n");


	/*********************************************
 	//Send/Receive messages to server
 	*********************************************/
	while (1) {

		//prepare input fields
		memset(sendBuffer, '\0', sizeof(sendBuffer));  //clear out buffer each time
		memset(clientMsg, '\0', sizeof(clientMsg));  //clear out message


		//get client input msg
		printf("%s> ", clientName);
		fgets(clientMsg, sizeof(clientMsg)-1, stdin);
		fflush(stdin);

		//check for "quit" then send, if called for
		if (strcmp(clientMsg, "\\quit\n") == 0){
			printf("\nConnection Closed\n");
			exit(0);
		}
		else {
			strcat(sendBuffer, clientName);
			strcat(sendBuffer, "> ");
			strcat(sendBuffer, clientMsg);

			//send message to server
			charsWritten = send(socketFD, sendBuffer, strlen(sendBuffer), 0); //write to server
			if (charsWritten < 0) {
				printf("CLIENT: ERROR reading from socket");
				exit(1);
			}
			if (charsWritten < strlen(sendBuffer)){
				printf("CLIENT: WARNING: Not all data written to socket!\n");
			}
			
			//wait for message from server
			receiveMsg(socketFD);	
			fflush(stdin);
		}
	}
}


/***********************************************************
 * Function: receiveMsg()
 * Description: receives message from server
 * Parameters: socketFD
 * Pre-Conditions: chat started with server
 * Post-Conditions: message from server read
 * Author/Source: Ben Brewster (OSU prof) and Molly A
 * *********************************************************/
void receiveMsg(socketFD) {

	char recBuffer[512];
	int charsRead;

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
	if (strcmp(recBuffer, "") == 0) {
		printf("Server closed connection. Now Exiting\n");
		exit(0);
	}
	printf("%s \n", recBuffer);

}	







