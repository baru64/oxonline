#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

//Serwer domy�lny
#define SERVER "127.0.0.1"

/* Server's port number */
#define SERVPORT 2000

//wiadomosc przekazywana miedzy klientem a serwerem
typedef enum
{
  JOIN,			//pole name wewnatrz
  REFUSE,		//nic wewnatrz
  MOVE,			//pole move
  START,		//pole turn
  STATE,		//pole game_state
} type_t;		//

typedef enum
{
	win,
	lose,
	draw,
} state_t;

typedef struct msg
{
  uint8_t type;
  uint8_t len;

  union
  {
	char name[20];
	struct
	{
		uint8_t x;
		uint8_t y;
	} move;
    enum
    {
    	false,
    	true,
    } turn;
    state_t state;
  } data;

} message_t __attribute__ ((packed));

typedef enum
{
  BRAK    = 0,
  KOLKO   = 1,
  KRZYZYK = 2
} ruch_t;

typedef enum
{
  G_KOLKO   = 0,
  G_KRZYZYK = 1
} gracz_t;

typedef enum
{
  WYGRYWA_KOLKO     = -1,
  REMIS             =  0,
  WYGRYWA_KRZYZYK   =  1,
  NIEROZSTRZYGNIETA =  2
} rezultat_t;

typedef struct
{
  uint8_t pole[9];
} plansza_t;

void czysc_plansze(plansza_t *plansza)
{
  memset(plansza, BRAK, sizeof(plansza_t));
}

char pokaz_pole(ruch_t pole)
{
  char rezultat;
  switch (pole)
  {
    case KOLKO:
      rezultat = 'O';
    break;

    case KRZYZYK:
      rezultat = 'X';
    break;

    default:
      rezultat = '-';
    break;
  }
  return rezultat;
}

void pokaz_plansze(plansza_t *plansza)
{
  int x,y;
  for (y=0; y<3; y++)
  {
    for (x=0; x<3; x++)
    {
      printf("%c", pokaz_pole(plansza->pole[3*y+x]));
    }
    printf("\n");
  }
}

int zaznacz_ruch_n(plansza_t *plansza, gracz_t gracz, int nrPola)
{
  if (plansza->pole[nrPola] != BRAK)
    return 0;
  plansza->pole[nrPola] = (gracz == G_KOLKO) ? KOLKO : KRZYZYK;
  return 1;
}

int zaznacz_ruch(plansza_t *plansza, gracz_t gracz, int nrWiersza, int nrKolumny)
{
  nrWiersza--;
  nrKolumny--;
  int nrPola = 3*nrWiersza + nrKolumny;
  return zaznacz_ruch_n(plansza, gracz, nrPola);
}

int usun_ruch_n(plansza_t *plansza, int nrPola)
{
  if (plansza->pole[nrPola] == BRAK)
    return 0;
  plansza->pole[nrPola] = BRAK;
  return 1;

}

void zmien_gracza(gracz_t *gracz)
{
  *gracz = (*gracz == G_KOLKO) ? G_KRZYZYK : G_KOLKO;
}


//Argumenty a.out serverIpOrName serverPort
int main(int argc, char *argv[])
{
	int sd;
	int rc;
	int serverPort = SERVPORT;
	struct sockaddr_in serveraddr;
	char server[255];

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("B��d tworzenia gniazda");
		exit(-1);
	}
	else
		printf("Utworzono gniazdo klienta\n");

	//Jako argument podano adres serwera
	if (argc > 1)
		strcpy(server, argv[1]);
	else
		strcpy(server, SERVER);    //Use the default server name or IP

	if (argc > 2)
		serverPort = atoi(argv[2]);

	printf("��czenie z %s, port %d\n", server, serverPort);

	memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(serverPort);

	if ((serveraddr.sin_addr.s_addr = inet_addr(server)) == (unsigned long)INADDR_NONE)
	{
		struct hostent *hostp = gethostbyname(server);
		if (hostp == (struct hostent *)NULL)
		{
			printf("Nie znaleziono hosta -> ");
			printf("h_errno = %d\n", h_errno);
			printf("---This is a client program---\n");
			printf("Command usage: %s <server name or IP> <PORT>\n", argv[0]);
			close(sd);
			exit(-1);
		}
		memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
	}

	if ((rc = connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
	{
		perror("B��d po��czenia");
		close(sd);
		exit(-1);
	}
	else
		printf("Ustanowiono po��czenie\n");

    struct msg Bufor;
	printf("Podaj imie: ");
    char name1[20];
	scanf("%19s", name1);
    Bufor.type=JOIN;
    strcpy(Bufor.data.name, name1);
    send(sd, &Bufor, sizeof(Bufor), 0);

    recv(sd, &Bufor.type, sizeof(Bufor), 0);
    if (Bufor.type==REFUSE){
        perror("Serwer zajety");
        close(sd);
        exit(-1);
    }

    int nrWiersza;
    int nrKolumny;
    plansza_t mojaPlansza;
    czysc_plansze(&mojaPlansza);
    pokaz_plansze(&mojaPlansza);
    gracz_t aktGracz = G_KOLKO;
    gracz_t Gracz;
    if (Bufor.type==START && Bufor.data.turn==true){
        Gracz = G_KOLKO;
        printf("\tpodaj nr wiersza (1-3): ");
        scanf("%hhu", &Bufor.data.move.x);
        printf("\tpodaj nr kolumny (1-3): ");
        scanf("%hhu", &Bufor.data.move.y);
        Bufor.type=MOVE;
        send(sd, &Bufor, sizeof(Bufor), 0);
        zaznacz_ruch(&mojaPlansza, aktGracz, Bufor.data.move.x, Bufor.data.move.y);
        pokaz_plansze(&mojaPlansza);
    }
    else Gracz = G_KRZYZYK;

	recv(sd, &Bufor, sizeof(Bufor), 0);
    while (1){
        zaznacz_ruch(&mojaPlansza, aktGracz, Bufor.data.move.x, Bufor.data.move.y);
        pokaz_plansze(&mojaPlansza);
        printf("Ruch wykonuje %s\n", (aktGracz == G_KOLKO) ? "K�ko" : "Krzy�yk");
        printf("\tpodaj nr wiersza (1-3): ");
        scanf("%d", &nrWiersza);
        printf("\tpodaj nr kolumny (1-3): ");
        scanf("%d", &nrKolumny);
        if (zaznacz_ruch(&mojaPlansza, aktGracz, nrWiersza, nrKolumny) == 0)
            continue;
        zmien_gracza(&aktGracz);
        pokaz_plansze(&mojaPlansza);
		recv(sd, &Bufor, sizeof(Bufor), 0);
		if (Bufor.type == STATE)
			break;
    }

  printf("Koniec gry: ");
  if(Bufor.data.state==win){
      printf(" Wygrana");
  }
  else if(Bufor.data.state==draw){
      printf(" Remis");
  }
  else printf(" Przegrana");
  printf("\n");

  return 0;
}
