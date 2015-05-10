#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "AppObjects.h"
#include "../my_socket.h"
#include "../name_server.h"
#include "../common_functions.h"
#pragma comment (lib, "Ws2_32.lib")

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// variaveis globais
entrada tabela[SVNAME_TABELA_SIZE];
int pos_tabela[SVNAME_TABELA_SIZE];

void createEntry(char* nome, char* ip, char* porta){
	
	for (int i = 0; i < SVNAME_TABELA_SIZE; i++)
	{
		if (pos_tabela[i] == 0)
		{
			pos_tabela[i] = 1; // ocupado

			strcpy(tabela[i].nome, nome);
			strcpy(tabela[i].ip, ip);
			strcpy(tabela[i].porta, porta);

			break;
		}
	}
}

void printTable(){
	printf("Servidores de Arquivo Registrados...");
	for (int i = 0; i < SVNAME_TABELA_SIZE; i++)
	{
		if (pos_tabela[i] == 1)
		{
			printf("\n\t%d :: %s [%s:%s]",i, tabela[i].nome, tabela[i].ip, tabela[i].porta);
		}
	}
	printf("\n\n");
}

bool getEntry(char* nome, entrada* ent){

	for (int i = 0; i < SVNAME_TABELA_SIZE; i++)
	{
		if (pos_tabela[i] == 1)
		{
			if (strcmp(nome, tabela[i].nome) == 0){
				*ent = tabela[i];
				return true;
			}
		}
	}
	return false;
}

//DWORD WINAPI thread_Servidor(LPVOID lpParameter);
DWORD WINAPI thread_Servidor(LPVOID lpParameter)
{
	// Create a WSADATA object called wsaData.
	WSADATA wsaData;
	int iResult;
	SOCKET Server_Socket = INVALID_SOCKET;
	int count_socket = 0;
	// Declare an addrinfo object that contains a sockaddr structure
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int recvbuflen = DEFAULT_BUFLEN;
	
	struct sockaddr_storage their_addr;
	size_t addr_len;
	char s[INET6_ADDRSTRLEN];
	
	int flag_on = 1;              /* socket option flag */
	
	svname_msg msg;
	
	for (int i = 0; i < SVNAME_TABELA_SIZE; i++)
		pos_tabela[i] = 0;

	printf("Servidor de Nomes inicializando...\n\n");
	
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
	
	/* set reuse port to on to allow multiple binds per host */
	if ((setsockopt(Server_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&flag_on,
	       sizeof(flag_on))) < 0) {
	    perror("setsockopt() failed");
		exit(1);
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
	
	printf("\nAguardando mensagens...\n");

	while(true)
	{
		addr_len = sizeof their_addr;		
		checkResult( recvfrom(Server_Socket, (char *)&msg, (int)sizeof(msg), 0, (struct sockaddr *)&their_addr, (int *)&addr_len) );
				
		switch (msg.opcode)
		{
			case SVNAME_CREATE_ENTRY:		// registro
				printf("\t-->Inserir registro: %s [%s:%s].\n",
					msg.ent.nome,
					inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s),
					msg.ent.porta);
				
				strcpy(msg.ent.ip, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
				createEntry(msg.ent.nome, msg.ent.ip, msg.ent.porta);
				printTable();
				break;

			case SVNAME_REMOVE_ENTRY:		// de-registro
				printf("\t-->Remover registro '%s' ip[%s].\n", 
					msg.ent.nome,
					inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
				break;

			case SVNAME_CHECK_NAME:		// consulta
				printf("\t-->Cliente [%s] consultou'%s'.\n", 
					inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s),
					msg.ent.nome);
				
				msg.opcode = 0;
				entrada ent;
				if (getEntry(msg.ent.nome, &ent))
				{
					strcpy(msg.ent.nome, ent.nome);
					strcpy(msg.ent.ip, ent.ip);
					strcpy(msg.ent.porta, ent.porta);
					msg.opcode = 1;
				}
				
				iResult = sendto(Server_Socket, (const char *)&msg, (int)sizeof(msg), 0, (struct sockaddr *)&their_addr, addr_len);

				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(Server_Socket);
					WSACleanup();
					exit(1);
				}

				break;
		}
		
		Sleep(10);
	}
	closesocket(Server_Socket);
	return 0;
}
