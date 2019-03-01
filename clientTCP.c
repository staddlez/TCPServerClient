/******************************************************************
* Name: Christian Yap, , Alec Allain, Anthony Nguyen
* Date: 2-10-19
* Filename: clientTCP.c
* Description: Client side of TCP connection (multi-threaded server).
******************************************************************/
//Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <signal.h>

//Defines
#define PORTNUMBER 3333
#define MAXLINE 4096

//Globals
static volatile int keepRunning = 1;

/******************************************************************
* CTRL + C Handler
******************************************************************/
void intHandler(int dummy)
{
    keepRunning = 0;
    printf("Enter any character to kill this client...: ");
}

/******************************************************************
* connectInfo() Function
* Variables: char* - IP address of server ; int* - Port #
******************************************************************/
void connectInfo(char* inputAddress, int* inputPort)
{
		
	char inputP[5];

	printf("CONNECT to a Server: Enter server IP address (Default 127.0.0.1): ");
	scanf("%s", &inputAddress[0]);
	printf("Enter Port # (Default 3333): ");
	fgets(inputP, sizeof(inputP), stdin);
	fgets(inputP, sizeof(inputP), stdin);		//Need two for some reason; fgets getting skipped even with fflush
	*inputPort = atoi(inputP);
}


/******************************************************************
* Main Function Program
******************************************************************/
int main()
{
    int clientSocket, status, endClient = 0;
    struct sockaddr_in serverAddress;
    char buffer[MAXLINE];
	char inputAddress[16] = "127.0.0.1";
	int inputPort = PORTNUMBER;
	char input[2] = "0";
	//char* pos;
	char fileSizeBuffer[256];
	int n;

	
	//SIGNAL REGISTER
	signal(SIGINT, intHandler);
	
	while(!endClient)
	{
		//See if they want to connect or exit:
		printf("1 - Connect to Default Server 127.0.0.1:3333\n2 - Input Server Info\n3 - Exit Client\nYour input: ");
		
		while(input[0] <= '0')
		{
			fgets(input,sizeof(input),stdin);
		}

		if(input[0] == '3')
		{
			exit(1);
		}
			
		//Get Connection info
		if(input[0] ==  '2')
		{
			connectInfo(inputAddress, &inputPort);
		}

		fflush(stdin);
		
		printf("CONNECTING TO %s PORT %d...\n", inputAddress, inputPort);
		clientSocket = socket(AF_INET, SOCK_STREAM, 0);

		//if error, exit program:
		if(clientSocket < 0)
		{
			printf("[-]Error. Unable to connect.\n");
			exit(-1);
		}

		//Else socket was created successfully...
		printf("Client socket successfully created!\n");

		//Separate values with null
		memset(&serverAddress, '\0', sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(inputPort);
		serverAddress.sin_addr.s_addr = inet_addr(inputAddress); //Use local IP
		
		status = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	
		//If error in connecting, exit program:
		if(status < 0)
		{
			printf("[-]Error. Unable to connect.\n");
			exit(-1);
		}
		
		//Else we successfully connected:
		printf("Client successfully connected to server.\n");

		input[0] = '0';
    
		//While loop to keep client running while connected to server.
		do
		{		
			printf("Client to Server: ");			//ADD YOUR COMMANDS HERE @GROUP
			//scanf("%s", &buffer[0]);
			fgets(buffer, 255, stdin);
			n = strlen(buffer);
  
			if(n > 0 && buffer[n-1] == '\n'){ //line break
				buffer[n-1] = '\0';
			}
			
			//Remove newline:
			//buffer[strcspn(buffer, "\n")] = '\0';

			//If kill client:
			if(!keepRunning)
			{
				if(write(clientSocket,"quit",strlen("quit")) == -1)
				{
					printf("Error. Unable to send message: %s \n",strerror(errno));
					close(clientSocket);
					exit(1);
				}
				printf("Disconnected successfully!\n");
				close(clientSocket);
				endClient = 1;
				exit(1);
			}
					
			//Send message to server:
			if(write(clientSocket,buffer,strlen(buffer)) == -1)
			{
				printf("Error. Unable to send message: %s \n",strerror(errno));
				close(clientSocket);
				exit(1);
			}
			
			//Upload
			if (buffer[0] == 'u' &&
				buffer[1] == 'p' &&
				buffer[2] == 'l' &&
				buffer[3] == 'o' &&
				buffer[4] == 'a' &&
				buffer[5] == 'd') {
				printf("Uploading to server...\n");
				n = -1;
				//wait for the server's ACK
                n = recv(clientSocket, buffer, sizeof(buffer), 0);
                if(n < 0) {
                    printf("Server didn't acknowledge name\n");
				} else {
					printf("Server ACK'd.\n");
				}
				
				//Parsing input
				char fn[256];
				int j = 0;
				for(int i = 7; i <= strlen(buffer)-1; i++)
				{
					fn[j] = buffer[i];
					j++;
					//printf("bf: %c\n", buffer[i]);
				}	
				fn[j] = '\0';
				
				int errnum;
				
				//open file
                FILE* fp;
				fp = fopen(fn, "rb"); //filename, read bytes
				if(fp == NULL){
					printf("error opening file in: %s\n", fn);
					errnum = errno;
					fprintf(stderr, "Value of errno: %d\n", errno);
					perror("Error printed by perror");
					fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
				} else {
					printf("File opened successfully!\n");
				}
				
				//figure out file size:
				int file_size = 0;
				if(fseek(fp, 0, SEEK_END) != 0)
				printf("Error determining file size\n");

				file_size = ftell(fp);
				rewind(fp);
				printf("File size: %d bytes\n", file_size);
				
				memset(&fileSizeBuffer, 0, sizeof(fileSizeBuffer));
				sprintf(fileSizeBuffer, "%d", file_size);
				
				//send file size
				n = send(clientSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
				if(n < 0) {
					printf("Error sending file size information\n");
				} else {
					printf("fileSizeBuffer: %s\n", fileSizeBuffer);
				}
				
				//receive ack for file size
				n = recv(clientSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
                if(n < 0) {
                    printf("Error receiving handshake\n");
				} else {
					printf("file size ack get.\n");
				}
				
				//create byte array
				char byteArray[256];
				memset(&byteArray, 0, sizeof(byteArray));
				
				int buffRead = 0;
                int bytesRemaining = file_size;
				
				while(bytesRemaining != 0)
			    {
				   //we fill in the byte array
				   //with slabs smaller than 256 bytes:
				   if(bytesRemaining < 256)
				   {
					   buffRead = fread(byteArray, 1, bytesRemaining, fp);
					   bytesRemaining = bytesRemaining - buffRead;
					   n = send(clientSocket, byteArray, 256, 0);
					   if(n < 0){
							   printf("Error sending small slab\n");
					   }

					   printf("sent %d slab\n", buffRead);
				   }
				   //for slabs of 256 bytes:
				   else
				   {
					   buffRead = fread(byteArray, 1, 256, fp);
					   bytesRemaining = bytesRemaining - buffRead;
					   n = send(clientSocket, byteArray, 256, 0);
					   if(n < 0)
							   printf("Error sending slab\n");
					   printf("sent %d slab\n", buffRead);
				   }
			    }
			    printf("File sent!\n");
			    //clean buffers
			    memset(&buffer, 0, sizeof(buffer));
			    memset(&byteArray, 0, sizeof(byteArray));
			    fclose(fp);
			}
			
			if (buffer[0] == 'r' &&
			    buffer[1] == 'e' &&
			    buffer[2] == 't' &&
			    buffer[3] == 'r' &&
			    buffer[4] == 'i' &&
			    buffer[5] == 'e' &&
			    buffer[6] == 'v' &&
			    buffer[7] == 'e') {	
				
				int fileSize;
				
				printf("Starting Retrieve process...\n");
				
				n = -1;
				//wait for the server's ACK
                n = recv(clientSocket, buffer, sizeof(buffer), 0);
                if(n < 0) {
                    printf("Server didn't acknowledge name\n");
				} else {
					printf("Server ACK'd.\n");
				}
				
				
				char fName[256];
				int j = 0;
				for(int i = 9; i <= strlen(buffer)-1; i++)
				{
					fName[j] = buffer[i];
					j++;
					//printf("bf: %c\n", buffer[i]);
				}
				fName[j] = '\0';

				printf("...lets get buffersize...\n");
				
				/* Receiving file size */
				n = recv(clientSocket, buffer, MAXLINE, 0);
				fileSize = atoi(buffer);
				
				if(n < 0) {
					printf("Error receiving file size\n");
				} else {
					printf("File size : %d\n",fileSize);
				}
				
				//Send ACK for File Size
				n = send(clientSocket, buffer, sizeof(buffer), 0);
				if(n < 0){
					printf("Error sending ACK for file size\n");
				} else {
					printf("File Size ACK sent.\n");
				}

				printf("...Now open new file...\n");
				

				
				FILE *receivedFile;
				int remainData = 0;
				int readData = 0;
				int i = 0;
				remainData = fileSize;
				ssize_t len;
				receivedFile = fopen(fName, "wb");
				
				printf("...Incoming File Size: %d ...\n",remainData);
				printf("...Lets read the sentfile...\n");

				for(remainData = fileSize;  remainData > 0;) {
					if(remainData < 256){
						len = recv(clientSocket, buffer, remainData, 0);
						fwrite(buffer, sizeof(char), len, receivedFile);
						remainData -= len;
						printf("Received %lu bytes, expecting %d bytes\n", len, remainData);
						break;
					} else{
						len = recv(clientSocket, buffer, 256, 0);
						fwrite(buffer, sizeof(char), len, receivedFile);
						remainData -= len;
						printf("Received %lu bytes, expecting: %d bytes\n", len, remainData);
					}
				}
				fclose(receivedFile);
					
				
				printf("...finished reading file...\n");
				
				memset(&buffer, 0 , sizeof(buffer));
				printf("...file closed...\n");
			}
			
			//Quit message
			if(strcmp(buffer, "quit") == 0)
			{
				printf("Disconnected from server.\n");
				close(clientSocket);
				break;
			}

			//List message
			if (strcmp(buffer, "list") == 0) {
				printf("Files in server directory:\n");

				if (recv(clientSocket, buffer, MAXLINE, 0) < 0) {
					printf("Unable to list directory data\n");
				}
				else {
					printf("%s\n", buffer);
				}
			//	break;
			}
			
		} while(strcmp(buffer, "exit") != 0);

	}
}

