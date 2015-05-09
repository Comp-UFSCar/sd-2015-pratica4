#define DEFAULT_BUFLEN 512


typedef struct
{
  char ip[10];
  char port[6];
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

