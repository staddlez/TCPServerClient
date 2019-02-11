/******************************************************************
* Name: Christian Yap, ,
* Date: 2 - 10 -19
* Filename: serverTCP.c
* Description: Server side of TCP connection (multi-threaded server).
******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 3333

//Globals
pid_t childPID = -1;

int main(){

	int socketCreate, status;
	int newSocket;
	char buffer[1024];
	struct sockaddr_in serverAddress;
	struct sockaddr_in newAddress;
	socklen_t addrSize;

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
	while(1){
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
			//A while loop within the child, wild!!
			while(1)
			{
				//Receive info from client...
				recv(newSocket, buffer, 1024, 0);
				
				//If client quits...
				if(strcmp(buffer, "quit") == 0)
				{
					printf("Disconnected from client %s:%d\n", inet_ntoa(newAddress.sin_addr), ntohs(newAddress.sin_port));
                    			break;
                    			//kill(childPID, SIGKILL);   Kill process commented out - kills entire server.....used break for now.
				}
				else
				{
					printf("Client: %s\n", buffer);
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
			}
		}

	}

	close(newSocket);


	return 0;
}
