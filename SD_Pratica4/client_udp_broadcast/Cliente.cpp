
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "AppObjects.h"
#include "..\my_socket.h"
#include <string>
#include <cstdio>
#include <iostream>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

DWORD WINAPI thread_Cliente(LPVOID lpParameter);

DWORD WINAPI thread_Cliente(LPVOID lpParameter)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
	                *ptr = NULL,
	                hints;
	
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	message msg1;
	unsigned char mc_ttl=1;     /* time to live (hop count) */
	
	SocketParams *params;
	params = (SocketParams *)lpParameter;
	
	printf("Entre com o mome do Cliente: \n");
	scanf("%s", &msg1.name);
	
	// Initialize Winsock  
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	         {
	          printf("WSAStartup failed with error: %d\n", iResult);
				 return 1;
		        }
	
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = params->family;
	hints.ai_socktype = params->socktype;
	hints.ai_protocol = params->protocol;
	
	/* Resolve the server address and port */
	iResult = getaddrinfo(params->ip, params->port, &hints, &ptr);
	if ( iResult != 0 ) {
		           printf("getaddrinfo failed with error: %d\n", iResult);
		           WSACleanup();
		           return 1;
	                     	}
	/* Create a SOCKET for connecting to server */
	ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ClientSocket == INVALID_SOCKET) 
		   {
		   printf("socket failed with error: %ld\n", WSAGetLastError());
		   WSACleanup();
		   return 1;
		     }
			  	
	/* set the TTL (time to live/hop count) for the send */
	if ((setsockopt(ClientSocket, SOL_SOCKET, SO_BROADCAST,
	       (const char *) &mc_ttl, sizeof(mc_ttl))) < 0) {
	    perror("setsockopt() failed");
	    exit(1);
	  } 
	  
	char status='A';
	while(status!='f')
	{
		printf("Enter the Client msg: \n");
		scanf("%s", &msg1.buf); 
				
		iResult = sendto(ClientSocket, (const char *)&msg1, (int)sizeof(msg1), 0, ptr->ai_addr, ptr->ai_addrlen);
		
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		Sleep(10);
				
		printf("Tecle: f - Para terminar ou c - Para enviar outra mensagem \n");
				
		status='A';
		while ((status!='f') && (status!='c'))
		                {status=getchar();  Sleep(10);}
		
	}
	closesocket(ClientSocket);
	return 0;
}
