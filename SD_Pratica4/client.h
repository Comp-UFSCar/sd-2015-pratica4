#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include "my_socket.h"
#include "name_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

svname_msg buscaNome(char* nome)
{
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	printf("Buscando endereco do nome: %s... ", nome);

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

	SOCKET client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (client_socket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	/* set the TTL (time to live/hop count) for the send */
	unsigned char mc_ttl = 1;
	if ((setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&mc_ttl, sizeof(mc_ttl))) < 0) {
		perror("setsockopt() failed");
		exit(1);
	}

	svname_msg name_msg;
	strcpy(name_msg.ent.nome, nome);
	name_msg.opcode = SVNAME_CHECK_NAME;

	iResult = sendto(client_socket, (const char *)&name_msg, (int)sizeof(name_msg), 0, ptr->ai_addr, ptr->ai_addrlen);

	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(client_socket);
		WSACleanup();
		exit(1);
	}

	struct sockaddr_storage their_addr;
	size_t addr_len = sizeof their_addr;

	iResult = recvfrom(client_socket, (char *)&name_msg, (int)sizeof(name_msg), 0, (struct sockaddr *)&their_addr, (int *)&addr_len);
	if (iResult == -1) {
		printf("Erro-Close\n\n");
		exit(1);
	}

	closesocket(client_socket);

	printf("Finalizado.\n");
	return name_msg;
}

#endif