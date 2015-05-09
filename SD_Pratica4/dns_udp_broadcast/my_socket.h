#define DEFAULT_BUFLEN 512


typedef struct
{
  char* ip;            /* multicast IP address */
  char port[6];
  int family;
  int socktype;
  int protocol;
  int flags;
} SocketParams;

typedef struct
{
  char name[15];
  int nro_msg;
  char buf[DEFAULT_BUFLEN];
} message;

typedef struct
{
	char nome[30];
	char ip[30];
	char porta[6];
} entrada;

