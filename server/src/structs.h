//structs
#include "msg.h"
#define BUF_LEN 10
#define LIMIT 2

game GAME;
struct buffer IPCbuffer;
struct clientSession connections[LIMIT];

typedef struct game
{
	char board[9];
	enum
	{
		"O",
		"X",
		"draw",
		"none"
	} winner;
} game;


struct buffer
{
	int readIdx; //id odczytane
	int writeIdx; //id zapisane
	uint8_t player[BUF_LEN]; //numer gracza do ktorego ma zostac wyslana wiadomosc
	struct message_t messages[BUF_LEN];
};

struct clientSession
{
	pthread_t process;
	int notEmpty;
	int finished;
	int fd;
};

