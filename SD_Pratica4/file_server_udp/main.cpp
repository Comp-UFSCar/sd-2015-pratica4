/* main.cpp - Generated by Visual Multi-Thread Win32 */

#include <winsock2.h>
#include "AppObjects.h"
#include "../my_socket.h"
#include <stdio.h>
#include <stdlib.h>
#define DEFAULT_PORT "27015"


extern DWORD WINAPI thread_Servidor(LPVOID lpParameter);
void main(void);

void main(void)
{
	SocketParams params;

	char port[6];
	char name[30];

	printf("Configuracoes do Servidor de Arquivo\n");
	printf("NOME: ");
	scanf("%s", &name);

	printf("PORTA: ");
	scanf("%s", &port);
	
	strcpy(params.port, port);
	strcpy(params.name, name);

	params.family = AF_INET;
	params.socktype = SOCK_DGRAM;
	params.protocol= IPPROTO_UDP;
	params.flags = AI_PASSIVE;
	Servidor = CreateThread(0, 0,(LPTHREAD_START_ROUTINE) thread_Servidor, &params, 0, 0);
	WaitForSingleObject(Servidor, INFINITE);
}
