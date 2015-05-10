#ifndef NAME_SERVER_H
#define NAME_SERVER_H

#define SVNAME_CREATE_ENTRY 1
#define SVNAME_REMOVE_ENTRY 2
#define SVNAME_CHECK_NAME 3

#define SVNAME_PORT "7000"
#define SVNAME_TABELA_SIZE 30

typedef struct 
{
	char nome[30];
	char ip[30];
	char porta[6];
} entrada;

typedef struct
{
	entrada ent;
	int opcode;
} svname_msg;

#endif