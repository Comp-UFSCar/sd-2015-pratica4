#ifndef FILE_SERVER_H
#define FILE_SERVER_H

#define CREATE 1
#define READ 2
#define WRITE 3
#define DELETA 4

#include <winsock2.h>
#include <ws2tcpip.h>
#include "my_socket.h"
#include "name_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

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

void BroadcastNome(entrada *e)
{
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	printf("Enviando informacoes do Servidor de Arquivos por Broadcast... ");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int iResult = getaddrinfo("255.255.255.255", SVNAME_PORT, &hints, &ptr);

	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	SOCKET Server_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (Server_Socket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	/* set the TTL (time to live/hop count) for the send */
	unsigned char mc_ttl = 1;
	if ((setsockopt(Server_Socket, SOL_SOCKET, SO_BROADCAST, (const char *)&mc_ttl, sizeof(mc_ttl))) < 0) {
		perror("setsockopt() failed");
		exit(1);
	}

	svname_msg name_msg;
	name_msg.ent = *e;
	name_msg.opcode = SVNAME_CREATE_ENTRY;

	iResult = sendto(Server_Socket, (const char *)&name_msg, (int)sizeof(name_msg), 0, ptr->ai_addr, ptr->ai_addrlen);

	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(Server_Socket);
		WSACleanup();
		exit(1);
	}

	closesocket(Server_Socket);

	printf("Finalizado.\n");
}

#endif