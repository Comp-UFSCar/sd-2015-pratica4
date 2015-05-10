#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "AppObjects.h"
#include "../my_socket.h"
#include "../common_functions.h"
#include "../name_server.h"
#include "../file_server.h"
#pragma comment (lib, "Ws2_32.lib")

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

DWORD WINAPI thread_Servidor(LPVOID lpParameter);

DWORD WINAPI thread_Servidor(LPVOID lpParameter)
{
	// Create a WSADATA object called wsaData.
	WSADATA wsaData;
	int iResult, iSendResult;
	SOCKET Server_Socket = INVALID_SOCKET;
	int count_socket=0;

	// Declare an addrinfo object that contains a sockaddr structure
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int recvbuflen = DEFAULT_BUFLEN;
	
	struct sockaddr_storage their_addr;
	size_t addr_len;
	char s[INET6_ADDRSTRLEN];
	
	message msg1, msg2;
	
	SocketParams *params;
	params = (SocketParams *)lpParameter;
	
	printf("\nServidor de Arquivos inicializando...\n\n");

	//Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
	
	entrada ent;
	strcpy(ent.nome, params->name);
	strcpy(ent.porta, params->port);
	strcpy(ent.ip, LOCALHOST);

	BroadcastNome(&ent);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = params->family;
	hints.ai_socktype = params->socktype;
	hints.ai_protocol = params->protocol;
	hints.ai_flags = params->flags;
	
	/* Resolve the server address and port */
	iResult = getaddrinfo(NULL, params->port, &hints, &result);
	
	if ( iResult != 0 ) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	
	/* Create a SOCKET
	   After initialization, a SOCKET object must be instantiated for use by the server */
	Server_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (Server_Socket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	
	/* Setup the UDP listening socket */
	iResult = bind(Server_Socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(Server_Socket);
		WSACleanup();
		return 1;
	}


	while(true)
	{
		printf("Aguardando operacao do cliente...\n");
		addr_len = sizeof(their_addr);
		
		iResult = recvfrom(Server_Socket, (char *)&msg1, (int)sizeof(msg1), 0, (struct sockaddr *)&their_addr, (int *) &addr_len);
		// Check for error
		if (iResult==-1)  printf("Erro-Close: %s\n\n",msg1.name); 
				
		printf("Nome do arquivo:  %s\n", msg1.name);
		printf("Opcode:  %d\n", msg1.opcode);
		
		int r = 0;
		// handler
		switch (msg1.opcode){
		case CREATE:
			r = do_create(&msg1, &msg2);
			break;
		case READ:
			r = do_read(&msg1, &msg2);
			break;
		case WRITE:
			r = do_write(&msg1, &msg2);
			break;
		case DELETA:
			r = do_delete(&msg1, &msg2);
			break;
		default:
			printf("Invalid code");
			break;
		}
		
		/* Echo the buffer back to the sender */
		iSendResult = sendto(Server_Socket, (const char *)&msg2, iResult, 0, (struct sockaddr *)&their_addr, (int)&addr_len);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(Server_Socket);
			WSACleanup();
			return 1;
		}

		Sleep(10);
	}
	printf("Cthulhu");
	closesocket(Server_Socket);
	return 0;
}