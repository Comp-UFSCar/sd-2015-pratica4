#define WIN32_LEAN_AND_MEAN
#define CREATE 1
#define READ 2
#define WRITE 3
#define DELETA 4

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "AppObjects.h"
#include "../my_socket.h"
#pragma comment (lib, "Ws2_32.lib")

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// prototipos
int do_create(message *, message *);
int do_read(message *, message *);
int do_write(message *, message *);
int do_delete(message *, message *);

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
	
	
	printf("Sistemas Distribuidos 2014: Servidor Inicializando ...\n\n");
	
	SocketParams *params;
	params = (SocketParams *)lpParameter;
	
	//Initialize Winsock
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
		printf("Servidor: waiting to recvfrom...\n");
		addr_len = sizeof(their_addr);
		
		iResult = recvfrom(Server_Socket, (char *)&msg1, (int)sizeof(msg1), 0, (struct sockaddr *)&their_addr, (int *) &addr_len);
		// Check for error
		if (iResult==-1)  printf("Erro-Close: %s\n\n",msg1.name); 
				
		printf("Server received-nome_arquivo:  %s\n", msg1.name);
		printf("Server received-opcode:  %d\n", msg1.opcode);
		printf("Bytes  received: %d\n", iResult);
		
		printf("Servidor: got packet from %s\n",inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
		printf("Servidor: packet is %d bytes long\n", iResult);

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
	return 0;
}

int do_create(message *m1, message *m2){
	FILE *fp;
	fp = fopen(m1->name, "w+");
	if (fp == NULL){
		strcpy(m2->buf, "Arquivo nao pode ser criado");
		return -1;
	}
	else {
		fclose(fp);
		strcpy(m2->buf, "Arquivo criado com sucesso!");
		printf("%s", "\nArquivo criado com sucesso!\n");
		return 0;
	}
}

int do_read(message *m1, message *m2){
	char buff[1024];
	FILE *fp;
	fp = fopen(m1->name, "r");
	if (fp == NULL){
		strcpy(m2->buf, "Nao foi possivel ler o arquivo");
		printf("\nNao foi possivel ler o arquivo\n");

		return -1;
	}
	else {
		fscanf(fp, "%s", buff);
		strcpy(m2->buf, buff);
		printf("\nString lida: %s\n", buff);
		fclose(fp);
		return 0;
	}
}

int do_write(message *m1, message *m2){
	FILE *fp;
	fp = fopen(m1->name, "a+");
	if (fp == NULL){
		strcpy(m2->buf, "Arquivo nao pode ser escrito!");
		return -1;
	}
	else {
		fprintf(fp, m1->buf);
		fclose(fp);
		printf("%s", "\nArquivo escrito com sucesso!\n");
		strcpy(m2->buf, "Arquivo escrito com sucesso!");
		return 0;
	}
}

int do_delete(message *m1, message *m2){
	int status;
	status = remove(m1->name);
	if (status == 0){
		printf("\nArquivo deletado com sucesso!\n");
		strcpy(m2->buf, "Arquivo deletado com sucesso!");
		return 0;
	}
	else {
		printf("\nNao foi possivel deletar o arquivo\n");
		strcpy(m2->buf, "Nao foi possivel deletar o arquivo");
		return -1;
	}
}