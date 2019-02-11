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

//Defines

#define PORTNUMBER 3333

//main function - Creates a new client socket.
int main()
{
    int clientSocket, status;
    struct sockaddr_in serverAddress;
    char buffer[1024];


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
	serverAddress.sin_port = htons(PORTNUMBER);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); //Use local IP
    
    status = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    //If error in connecting, exit program:
    if(status < 0)
    {
        printf("[-]Eror. Unable to connect.\n");
        exit(-1);
    }
    //Else we successfully connected:
    printf("Client successfully connected to server.\n");
    
    //While loop to keep client running while connected to server.
    while(1)
    {
        printf("Client Command: ");
        scanf("%s", &buffer[0]);
        
        if(strcmp(buffer, "quit") == 0)
        {
            close(clientSocket);
            printf("Disconnected from server.\n");
            exit(1);
        }
        
        //Receive from server:
        if(recv(clientSocket, buffer, 1024, 0) < 0)
        {
                printf("Unable to receive data. Try again.\n");
        }
        else
        {
                printf("Server Acknowledged. Received message: %s\n", buffer);
        }
    }




}

