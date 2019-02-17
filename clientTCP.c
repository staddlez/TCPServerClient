/******************************************************************
* Name: Christian Yap, ,
* Date: 2 - 10 -19
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

//KILL handler
void intHandler(int dummy)
{
    keepRunning = 0;
}


//main function - Creates a new client socket.
int main()
{
    int clientSocket, status;
    struct sockaddr_in serverAddress;
    char buffer[MAXLINE];
	char inputAddress[16] = "127.0.0.1";
	int inputPort = PORTNUMBER;
	char inputP[5], input[2] = "1";
	char* pos;
	
	//SIGNAL

	signal(SIGINT, intHandler);
	
	//See if they want to connect or exit:
	printf("1 - Connect to Default Server 127.0.0.1:3333\n2- Input Server Info\n3 - Exit Client\nYour input: ");
	//fgets(input,sizeof(input),stdin);

	if(input[0] == '3')
	{
		exit(1);
	}
		
	//Get Connection info
	if(input[0] ==  '2')
	{
		printf("CONNECT to a Server: Enter server IP address (Default 127.0.0.1): ");
		//fgets(input,sizeof(input),stdin);
		//fgets(inputAddress,sizeof(inputAddress),stdin);
		//printf("Enter Port # (Default 3333): ");
		//fgets(inputP, sizeof(inputP), stdin);
		//inputPort = atoi(inputP);
	}
	
	fflush(stdin);
	printf("CONNECTING TO %s PORT %d...\n", inputAddress, inputPort);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    //if error, exit program:
	if(clientSocket < 0){
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
    
    //While loop to keep client running while connected to server.
    while(1)
    {		
		printf("Client to Server: ");
        //scanf("%s", &buffer[0]);
		fgets(buffer,MAXLINE,stdin);
		
		//Remove newline:
		buffer[strcspn(buffer, "\n")] = '\0';
				
		//Send message to server:
		if(write(clientSocket,buffer,strlen(buffer)) == -1)
		{
			printf("Error. Unable to send message: %s \n",strerror(errno));
			close(clientSocket);
			exit(1);
		}
		
		//If kill client:
		if(!keepRunning)
		{
			if(write(clientSocket,"quit",strlen("quit")) == -1)
			{
				printf("Error. Unable to send message: %s \n",strerror(errno));
				close(clientSocket);
				exit(1);
			}
			
			close(clientSocket);
			exit(1);
		}
	
		//Quit message
		if(strcmp(buffer, "quit") == 0)
        {
            printf("Disconnected from server.\n");
			close(clientSocket);
            exit(1);
        }
        
        //Receive from server:
        if(recv(clientSocket, buffer, MAXLINE, 0) < 0)
        {
                printf("Unable to receive data. Try again.\n");
        }
        else
        {
				//If server goes down end connection:
				if(strcmp(buffer, "kill") == 0)
				{
					printf("Server down.\n");
					close(clientSocket);
					exit(1);
				}
                printf("Received from Server: %s\n", buffer);
        }
    }




}

