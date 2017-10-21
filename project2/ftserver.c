/***************************************************************
 * Name: Molly Arwood
 * Date: 5-18-17
 * Class: CS_372_NW
 * ftserver.c
 **************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h> //for pid stuff
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <netdb.h>


/************************************************
 * Prototypes
 * *********************************************/
void error(const char *msg) {perror(msg); exit(1); };
int startUp(char *);
int validateMsg(char *);
void sendDir(int, char*);
int getDirFiles(char *);
void deletePointerArray(char *);
int dynAllocMoreFileSpace(char *, int);
int dataConnect(int, struct sockaddr_in, int);
int checkFileExists(char *);
void extractFileAndSend(int, char *);


//global variables
int program = 1;

void main(int argc, char *argv[]) {

	int cntrlSocketFD, charsRead, cntrlConnectFD, dataSocketFD;
	char *portNumber = malloc(4);
	struct sockaddr_in  clientAddress;
	socklen_t sizeOfClientInfo;
	int msgSize = 1000;
	char *completeMsg = malloc(1000);
	char *command = malloc(4);
	char *dataPort = malloc(20);
	char *filename = malloc(50);
	char *clientHost = malloc(100);
	char *service = malloc(20);

	//check arguments
	if (argc < 2) {
		printf("USAGE: %s port_number \n", argv[0]);
	}

	//printf("port number: %s \n", argv[1]);
	portNumber = argv[1];

	//setup control connection
	cntrlSocketFD = startUp(portNumber);

	while (program) {

		//accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress);
		cntrlConnectFD = accept(cntrlSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);


		//confirm connection established with no error
		if (cntrlConnectFD < 0) {
			printf("ERROR on accept (server)\n");
			exit(1);
		}

		//Read the client's command
		memset(completeMsg, '\0', msgSize);
		charsRead = recv(cntrlConnectFD, completeMsg, msgSize - 1, 0);
		if(charsRead < 0) {
			printf("ERROR reading from socket\n");
			exit(1);
		}

		//check client command is valid
		char *msg = malloc(100);;
		strcpy(msg, completeMsg);
		int valid = validateMsg(msg);	


		/******************************************
		//valid = 2 means -l command
		******************************************/
		if (valid == 2) {

			//assign pointers;		
			command = strtok_r(completeMsg, "|",&completeMsg);
			dataPort = strtok_r(NULL,"|", &completeMsg);

			//innitiate data connection with client
			printf("Opening data connection with client\n");
			int dataPortInt = atoi(dataPort);
			dataSocketFD = dataConnect(dataPortInt, clientAddress, cntrlConnectFD);

			//get dir file names and number of files
			int fileNum = 0;
			char *dirFiles = malloc(500 * sizeof(char));
			fileNum =  getDirFiles(dirFiles);
	
			//send server's dir to client
			sendDir(dataSocketFD, dirFiles);

			//free memory
			sleep(1);
			deletePointerArray(dirFiles);
			printf("Connection Closed With Client\n");
			
		}
		/*************************************
		//valid = 1 means -g command
		*************************************/
		else if (valid == 1) {

			//break down original msg
			command = strtok_r(completeMsg, "|", &completeMsg);
			dataPort = strtok_r(NULL, "|", &completeMsg);
			filename = strtok_r(NULL, "|", &completeMsg);

			printf("dataPortG: %s\n", dataPort);	
			printf("filenameG: %s\n", filename);
			//check if file exists
			int exists = checkFileExists(filename);
			if (exists == 1) {

				//initiate data connection with client
				printf("Opening data connection with client\n");
				int dataPortInt = atoi(dataPort);
				dataSocketFD = dataConnect(dataPortInt, clientAddress, cntrlConnectFD);

				//extract file contents and send
				extractFileAndSend(dataPortInt, filename);

			}
			else {
				//send error message to client
				char errMsg[30] = "ERROR: File Not Found\n";
				int charsWritten = send(cntrlConnectFD, errMsg, sizeof(errMsg), 0);
			}

			printf("Connection Closed With Client\n");
		}
		else {
			/**********************************
			//send error message to client
			**********************************/
			charsRead = send(cntrlConnectFD,"Invalid Command\n", 20, 0);
			int checkSend = -5;
			do {
				ioctl(cntrlConnectFD, TIOCOUTQ, &checkSend);
			} while (checkSend > 0);
			if (checkSend < 0) {
				printf("ioctl error\n");
				exit(2);
			}
		}


	}


}
 


/***************************************************************
 * Function: startUp
 * Description: sets up server listening/control socket 
 * Parameters:server port number
 * Pre-Conditions:
 * Post-Conditions:
 * Author: Ben Brewster (OSU prof)
 * ************************************************************/
int startUp(char *portNum){

	int controlSocketFD, portNumInt;
	struct sockaddr_in serverAddress;

	//set up address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); //clear out addr struct
	portNumInt = atoi(portNum); //convert port num to int
	serverAddress.sin_family = AF_INET; //creat network-capable socket
	serverAddress.sin_port = htons(portNumInt); //store port nume in big endian format
	serverAddress.sin_addr.s_addr = INADDR_ANY; //any addr is allowed to connect

	//set up the socket
	controlSocketFD = socket(AF_INET, SOCK_STREAM, 0); //create the socket
	if (controlSocketFD < 0) error("ERROR opening socket");

	//Enable socket to begin listening
	if (bind(controlSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) //connect socket to port
		error("ERROR on binding");
	listen(controlSocketFD, 5); //flip socket on - can now receive up to 5 cnnctns
	
	return controlSocketFD;
}



/***************************************************************
 * Function: dataConnect
 * Description: create/request data connection with client
 * Parameters: int port number
 * Pre-Conditions: control connection established. request sent
 * by client
 * Post-Condistions: data port created
 * Source/Author: Ben Brewster (OSU Prof)
 * ************************************************************/
int dataConnect(int dataPort, struct sockaddr_in clientAddress, int cntrlConnectFD) {
	int socketFD;
	int charsRead;
	char completeMsg[700];

	printf("Connecting to client...\n");
	sleep(1);

	//Read the client's command - looking for 'ready'
	memset(completeMsg, '\0', sizeof(completeMsg));
	charsRead = recv(cntrlConnectFD, completeMsg, sizeof(completeMsg), 0);
	
	if (strcmp(completeMsg, "ready") == 0) {

		/****************************************
		//set up the client address struct
		****************************************/
		clientAddress.sin_port = htons(dataPort);

		/*************0***************************
	 	//set up the socket
	 	****************************************/
		socketFD = socket(AF_INET, SOCK_STREAM, 0);
		if(socketFD < 0) {
			printf("SERVER: ERROR opening data socket\n");
			exit(1);
		}
	
		/****************************************
		//Connect to the client
		****************************************/
		if(connect(socketFD, (struct sockaddr*)&clientAddress, sizeof(clientAddress)) < 0) {
			printf("SERVER: ERROR connecting to client on data port\n");
			exit(2);
		}

		return socketFD;
	}
	else {
		printf("SERVER: Ready signal not recieved\n");
	}
}



/***************************************************************
 * Function: validateMsg
 * Description: determine whether client command is valid or not 
 * Parameters: char * with client's message
 * Pre-Conditions: control connection with client est. client
 * 	sent message to server
 * Post-Conditions: server confirms command is valid/invalid
 * Author: Molly
***************************************************************/
int validateMsg(char *msg) {

	printf("Validating Command...\n");
	char *cmd; 
	cmd = strtok_r(msg, "|",&msg);

	//check ommand
	if (strcmp(cmd, "-g") == 0){
		return 1;
	}
	else if (strcmp(cmd, "-l") != 0) {
		return 0;
	}
	else
		return 2;
}



/***************************************************************
 * Functionn: senDir
 * Description: sends message to client via data connection
 * Parameters: socket file descriptor, message/data
 * Pre-Conditions: control and data connect est. 
 * Post-Conditions: message sent to client
 * Author / Source: Molly
 * ************************************************************/
void sendDir(int dataSocketFD, char *files) {
	printf("Sending File Listing to Client...\n");

	char sendBuffer[700];
	int charsWritten;

	memset(sendBuffer, '\0', sizeof(sendBuffer));
	strncpy(sendBuffer, files, sizeof(sendBuffer - 1));

	charsWritten = send(dataSocketFD, sendBuffer, strlen(sendBuffer), 0);
	if (charsWritten < 0) {
		printf("SERVER: ERROR reading from socket during data conn send\n");
		exit(1);
	}
	printf("File Listing Transfer Complete\n");
}




/***************************************************************
 * Function:getDirFiles()
 * Description: loops through files in cwd and places them in 
 * 	string array. also counts number of files.
 * Parameters: pointer to array of pointers to filenames
 * Pre-Conditions: 
 * Post-Conditions: array filled with filenames. num of files
 * 			returned
 * Author / Source: Molly
 * ************************************************************/
int getDirFiles(char *files) {

	printf("Retrieving Files...\n");
	int fileNum = 0;
	DIR *d;
	struct dirent *dir;
	d = opendir(".");

	//check dir was opened
	if (d == NULL){
		printf("Error opening directory\n");
		return fileNum;
	}

	//read files into char* and count instances
	while ((dir = readdir(d)) != NULL){
		if (dir->d_type == DT_REG) {

			strcat(files, dir->d_name);
			strcat(files,"|");

			fileNum++;
		}
	}

	//remove last pipe
	files[strnlen(files, 1000) - 1] = '\0';

	return fileNum;
}



/***************************************************************
 * Function: checkFileExists
 * Description: check to see if file is in directory
 * Parameters: filename
 * Pre-Conditions: control connection est.
 * Post-Conditions: determined if file exists
 * Souorce/Author: Molly
 * ************************************************************/
int checkFileExists(char *filename) {
	printf("Looking for Requested File...\n");
	sleep(1);	

	DIR *d;
	struct dirent *dir;
	d = opendir(".");

	printf("filename2: %s\n", filename);
	//check dir was opened
	if (d == NULL) {
		printf("Error opening directory\n");
		return 0;
	}

	//read files and check for match
	while ((dir = readdir(d)) != NULL) {
		if (strcmp(dir->d_name, filename) == 0) {
			return 1;
		}
	}
	
	return 0;
}



/***************************************************************
 * Function:
 * Description:
 * Parameters:
 * Pre-Conditions:
 * Post-Conditions:
 * Author/Source: stackoverflow.com/questions/174531/easiest-way
 * 	-to-get-files-contents-in-c
 * ************************************************************/
void extractFileAndSend(int dataPort, char *filename) {
	printf("Sending File Contents to Client...\n");
	sleep(1);

	FILE *fp = fopen(filename, "r");
	char buffer[200];	
	int charsWritten;

	memset(buffer, '\0', sizeof(buffer));

	if (fp) {
		while (fgets(buffer, sizeof(buffer), fp)) {
			printf(buffer);
			charsWritten = send(dataPort, buffer, sizeof(buffer),0);
			if (charsWritten < 0) {
				break;
			}
		}
		charsWritten = send(dataPort, "%!?%", 5, 0);
	}
	else {
		printf("SERVER: ERROR opening file\n");
	}
	printf("Transfer Complete\n");
}




/***************************************************************
 * Function: dynAllocMoreFileSpace
 * Descriptioin: gives var more dyn allocated memory
 * Parameters: char **, int
 * Pre-Conditions: var running out of memory
 * Post-Conditions: var has more memory
 * Author/Source: Molly 
 * ************************************************************/
int dynAllocMoreFileSpace(char *files, int curSize) {

	char *temp = malloc(curSize*2 * sizeof(char));

	//transfer file names over
	strcpy(temp, files);
	
	deletePointerArray(files);
	files = temp;

	return curSize*2;

}

/***************************************************************
 * Function: deletePointerArray
 * Description: deletes dynamically allocated char * vars
 * Parameters: char *
 * Pre-Conditions: variable created
 * Post-Conditions: variable memory freed
 * Author/Source: Molly 
 * ************************************************************/
void deletePointerArray(char *file) {

	if (file != NULL)
		free(file);

}



