/******************************************************************
* Name: Christian Yap, Anthony Nguyen, Alec Allain, Joe Dubois
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
#include <sys/stat.h>
#include <sys/sendfile.h>

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
	char fileSizeBuffer[256];
	DIR *directory;
	struct dirent *dir;
	directory = opendir("./");
	
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
                    //n = recv(newSocket, buffer, 256, 0);
                    //clean buffer
					memset(&buffer, 0 , sizeof(buffer));
				}
				
				
				
				else if(buffer[0] == 'r' &&
						buffer[1] == 'e' &&
					    buffer[2] == 't' &&
					    buffer[3] == 'r' &&
					    buffer[4] == 'i' &&
					    buffer[5] == 'e' &&
					    buffer[6] == 'v' &&
					    buffer[7] == 'e') {
					
					printf("...client wants to retrieve file...\n");
					
					n = send(newSocket, buffer, sizeof(buffer), 0);
                    if(n < 0) {
                        printf("Error sending retrieve ACK\n");
					} else {
						printf("File Name get\n");
					}
					char fileToSend[256];
					
					int j = 0;
					for(int i = 9; i <= strlen(buffer)-1; i++)
					{
						fileToSend[j] = buffer[i];
						j++;
						//printf("bf: %c\n", buffer[i]);
					}
					fileToSend[j] = '\0';
					
				    //struct stat file_stat;
					ssize_t len;    
					
					//open file
					FILE* fd;
					fd = fopen(fileToSend, "rb");
                    if(fd == NULL) {
						printf("Error opening file...\n");
					} else {
						printf("...open local file...\n");
					}
					
					printf("...get file stats...\n");

        			/* Get file stats */
				    int file_size = 0;
					if(fseek(fd, 0, SEEK_END) != 0)
					printf("Error determining file size\n");

					file_size = ftell(fd);
					rewind(fd);
					printf("File size: %d bytes\n", file_size);
					
					memset(&fileSizeBuffer, 0, sizeof(fileSizeBuffer));
					sprintf(fileSizeBuffer, "%d", file_size);
					
					printf("...send file size to client..,\n");

					/* Sending file size */
					len = send(newSocket, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
					if (len < 0) {
						fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));
					} else {
						fprintf(stdout, "Server sent %ld bytes for the size\n", len);
					}
     
					//receive ack for file size
					len = recv(newSocket, fileSizeBuffer, sizeof(fileSizeBuffer),0);
					if(len < 0) {
						printf("...Error receiving handshake...\n");
					} else {
						printf("...Received file size ack...\n");
					}
						
					//create byte array
					char byteArray[256];
					memset(&byteArray, 0, sizeof(byteArray));
				
					int buffRead = 0;
					int bytesRemaining = file_size;
					
					printf("...now send file data to client...\n");	
					/* Sending file data */
					while(bytesRemaining != 0)
					{
					   //fill in the byte array
					   //with segments smaller than 256 bytes:
					   if(bytesRemaining < 256)
					   {
						   buffRead = fread(byteArray, 1, bytesRemaining, fd);
						   bytesRemaining = bytesRemaining - buffRead;
						   n = send(newSocket, byteArray, 256, 0);
						   if(n < 0){
								   printf("Error sending small slab\n");
						   }

						   printf("sent %d slab\n", buffRead);
					   }
					   //for segments of 256 bytes:
					   else
					   {
						   buffRead = fread(byteArray, 1, 256, fd);
						   bytesRemaining = bytesRemaining - buffRead;
						   n = send(newSocket, byteArray, 256, 0);
						   if(n < 0)
								   printf("Error sending slab\n");
						   printf("sent %d slab\n", buffRead);
					   }
					}
				
					printf("...client has recieved file...\n");
					memset(&buffer, 0, sizeof(buffer));
					memset(&byteArray, 0, sizeof(byteArray));
			        fclose(fd);
				}
				
				//If client wants directory listing
				//else if (strcmp(buffer, "list") == 0) {
					//printf("Recieved '%s' from client", buffer);

					//char **names, **temp;
					//size_t i, size = 1;
					//DIR *d = opendir(".");
					//if (d == NULL) {
					//	fprintf(stderr, "Cannot open directory...\n");
					//}
					//else if (d) {
					//	while ((dir = readdir(d))!= NULL) {
					//		names[i] = dir->d_name;
					//		i++;
					//		if (i > size) {
					//			temp = realloc(names, size*2*sizeof(*names));
					//			names = temp;
					//			size = size * 2;
					//		}
					//	}
					//}

					//closedir(d);
					//strncpy(buffer, *names, MAXLINE);
					//send(newSocket, buffer, strlen(buffer), 0);
					//bzero(buffer, sizeof(buffer));
				if (strcmp(buffer, "list") == 0) {
					printf("Client asking for directory listing\n");
					
					char filelist[4096];
					
					//if dir opens properly
					if (directory) {
						
						while ((dir = readdir(directory)) != NULL) {
							
							//if buffer is empty
							if (sizeof(filelist) == 0) {
								//ignore . and ..
								if (strcmp(dir->d_name, ".") == 0 ||
									strcmp(dir->d_name,"..") == 0) {
										// do nothing
									} else {
										printf("\n%s", dir->d_name);
										sprintf(filelist, "\n%s", dir->d_name);
									}
									
							} else {
								//ignore . and ..
								if (strcmp(dir->d_name, ".") == 0 ||
									strcmp(dir->d_name,"..") == 0) {
										//do nothing
									} else {
										printf("\n%s", dir->d_name);
										sprintf(filelist+strlen(filelist), "\n%s", dir->d_name);
									}
								
							}
						}
						//Send when all files are accounted for
						n = send(newSocket, filelist, sizeof(filelist), 0);
						
						if (n < 0) {
							printf("Failed to send list.\n");
						}
						
					} else {
						//Could not open directory
						sprintf(filelist, "server could not open directory"); 
						
					}
					
					//clear filelist and buffer
					memset(filelist, 0, sizeof(filelist));
					memset(buffer, 0, sizeof(buffer));
				}
				
				//If client quits...
				else if(strcmp(buffer, "quit") == 0)
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
