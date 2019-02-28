/******************************************************************
* Name: Christian Yap, , Alec Allain,
* Date: 2-10-19
* fn: serverTCP.c
* Description: Server side of TCP connection (multi-threaded server).
******************************************************************/
//Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>

//Defines
#define PORT 3333
#define MAXLINE 4096

//Globals
pid_t childPID = -1;

/****************************************************************
 * Main Function Program
 ***************************************************************/
int main(){

	int socketCreate, newSocket, status, fileSize;
	char buffer[MAXLINE];
	struct sockaddr_in serverAddress;
	struct sockaddr_in newAddress;
	socklen_t addrSize;
	struct dirent *dir;
	char fileSizeBuffer[256];

	//Create a new socket:
	socketCreate = socket(AF_INET, SOCK_STREAM, 0);
	
	//If socket fails:
	if(socketCreate < 0)
	{
		printf("Error. Unable to connect.\n");
		exit(1);
	}
	//Else server socket is created successfully:
	printf("Server socket successfully created.\n");

	//Server information. Run locally:
	memset(&serverAddress, '\0', sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Let's start the server...
	status = bind(socketCreate, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	//If failure...such as port being blocked...
	if(status < 0)
	{
		printf("Failure to bind server. Try changing port...\n");
		exit(1);
	}
	printf("Server binded to port %d\n", 3333);

	//Start listening:
	if(listen(socketCreate, 10) == 0)
	{
		printf("Now Listening....\n");
	}
	else
	{
		printf("Error in binding.\n");
	}

	//While loop to keep server running:
	while(1)
	{
		//Create new socket once client tries to connect:
		newSocket = accept(socketCreate, (struct sockaddr*)&newAddress, &addrSize);
		if(newSocket < 0)
		{
			exit(1);
		}
		//Successful in connection:
		printf("Connection accepted with Client %s:%d\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port));
				
		//Fork for new client:
		if((childPID = fork()) == 0)
		{
			close(socketCreate);
			int n;
			
			//A while loop within the child, wild!!
			while(1)
			{				
				memset(&buffer, 0, sizeof(buffer));
				//Receive info from client...
				n = recv(newSocket, buffer, sizeof(buffer), 0);
				
				if (n < 0) {
					printf("Can't receive from client");
				} else {
					buffer[n] = '\0';
				}
				
				printf("b0 %c\n", buffer[0]);
				if (buffer[0] == 'u' &&
					buffer[1] == 'p' &&
					buffer[2] == 'l' &&
					buffer[3] == 'o' &&
					buffer[4] == 'a' &&
					buffer[5] == 'd') {
				
					printf("Client Uploading...\n");
					
					n = send(newSocket, buffer, sizeof(buffer), 0);
                    if(n < 0) {
                        printf("Error sending file ACK\n");
					} else {
						printf("File Name get\n");
					}
					
					//Receive File Size
					memset(&fileSizeBuffer, 0, sizeof(fileSizeBuffer));
					n = recv(newSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
                    if(n < 0) {
						printf("Error receiving file size\n");
					} else {
						printf("size should be: %s\n", fileSizeBuffer);
					}
					
					//Send ACK for File Size
					n = send(newSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
                    if(n < 0){
						printf("Error sending ACK for file size\n");
					} else {
						printf("File Size ACK sent.\n");
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
					fileSize = atoi(fileSizeBuffer);
					
					//Writing file
					memset(&buffer, 0, sizeof(buffer));
					int remainingData = 0;
					ssize_t len;
					FILE* fp;
					fp = fopen(fn, "wb");
					remainingData = fileSize;
					while(remainingData != 0) {
                        if(remainingData < 256){
                            len = recv(newSocket, buffer, remainingData, 0);
                            fwrite(buffer, sizeof(char), len, fp);
                            remainingData -= len;
                            printf("Received %lu bytes, expecting %d bytes\n", len, remainingData);
							break;
                        } else{
                            len = recv(newSocket, buffer, 256, 0);
                            fwrite(buffer, sizeof(char), len, fp);
                            remainingData -= len;
                            printf("Received %lu bytes, expecting: %d bytes\n", len, remainingData);
                        }
					}
					fclose(fp);
					//catch weird last packet
                    n = recv(newSocket, buffer, 256, 0);
                    //clean buffer
					memset(&buffer, 0 , sizeof(buffer));
				}
				
				//If client wants directory listing
				if (strcmp(buffer, "list") == 0) {
					//printf("Recieved '%s' from client", buffer);

					char **names, **temp;
					size_t i, size = 1;
					DIR *d = opendir(".");
					if (d == NULL) {
						fprintf(stderr, "Cannot open directory...\n");
					}
					else if (d) {
						while ((dir = readdir(d))!= NULL) {
							names[i] = dir->d_name;
							i++;
							if (i > size) {
								temp = realloc(names, size*2*sizeof(*names));
								names = temp;
								size = size * 2;
							}
						}
					}
					closedir(d);

					strncpy(buffer, *names, MAXLINE);

					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
				
				//If client quits...
				if(strcmp(buffer, "quit") == 0)
				{
					printf("Disconnected from client %s:%d\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port));
					close(newSocket);
					break;
					//kill(childPID, SIGKILL);   Kill process commented out - kills entire server.....used break for now.
				}
				else
				{
					printf("Received message from Client %s:%d: %s\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port), buffer);
					//Server to Client: ACK message.
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
				
			}
		}
	}
	return 0;
}