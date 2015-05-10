#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#define DEFAULT_BUFLEN 512
#define LOCALHOST "127.0.0.1"

typedef struct
{
	char* ip;            /* multicast IP address */
	char port[6];
	char name[30];
	int family;
	int socktype;
	int protocol;
	int flags;
} SocketParams;

typedef struct
{
	char name[15];
	int opcode;
	char buf[DEFAULT_BUFLEN];
} message;

#endif