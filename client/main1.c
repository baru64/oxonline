
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
#include "msg.h"

//Serwer domy�lny
#define SERVER "127.0.0.1"

/* Server's port number */
#define SERVPORT 2000

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
    int msgLen;
    struct sockaddr_in serveraddr;
    char server[255];

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Blad tworzenia gniazda");
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

    printf("Laczenie z %s, port %d\n", server, serverPort);

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
        perror("Blad polaczenia");
        close(sd);
        exit(-1);
    }
    else
        printf("Ustanowiono polaczenie\n");


    message_t sendbuf;

    printf("Podaj imie: ");
    char player1[20];
    char player2[20];
    scanf("%19s", player1);
    sendbuf.type=JOIN;
    strcpy(sendbuf.data.name, player1);
    send(sd, &sendbuf, sizeof(sendbuf), 0);
    
    int game_end = 0;
    int joined = 0;
    int ssend = 0;
    plansza_t mojaPlansza;
    czysc_plansze(&mojaPlansza);
    gracz_t aktGracz = G_KOLKO;
    
    while(!game_end)
    {
    	message_t recvd;
    	message_t tosend;
    	//TODO recv
    	//wysylanie wiadomosci
    	//cofanie narysowaniego ostatnio ruchu gdy refuse
    	char* Bufor = (char*)malloc(1500);
    	memset( Bufor, 0, 1500 );
  		memset( &recvd, 0, sizeof(recvd) );
  		memset( &tosend, 0, sizeof(tosend) );
    	msgLen = recv(sd, Bufor, 1500, 0);
		memcpy(&recvd, Bufor, msgLen);
		char temp1[sizeof(recvd)];
        memcpy(temp1, &recvd, sizeof(recvd));
        for(int i = 0; i < sizeof(recvd); ++i) printf("%hhu ", temp1[i]);
		printf("\ntype: %hhu len: %hhu\n", recvd.type,recvd.len);
    	uint8_t x,y;
    	
    	switch(recvd.type)
    	{
    		case JOIN:
    		if(!joined)
    		{
    			strcpy(player2,recvd.data.name);
    			joined = 1;
    		}
    		break;
    		
    		case REFUSE:
    		if(!joined)
    		{
    			game_end = 1;
    			printf("Serwer jest zajety\n");
    		} 
    		else 
    		{
    			printf("Twoj ruch zostal odrzucony!\n");
    			//TODO tu zrobic dobrze
    		}
    		break;
    		
    		case START:
    		printf("otrzymano turn=%d", recvd.data.turn);
    		if(recvd.data.turn == true)
    		{
    			//printf("halko");
    			aktGracz = 0;
    			tosend.type = (uint8_t)MOVE; tosend.len = (uint8_t)4;
    			printf("Twoj ruch\n");
					printf("\tpodaj nr wiersza (1-3): ");
					scanf("%hhu", &tosend.data.move.x);
					printf("\tpodaj nr kolumny (1-3): ");
					scanf("%hhu", &tosend.data.move.y);
					tosend.data.move.x--;
					tosend.data.move.y--;
    			printf("sending: x:%hhu y:%hhu\n", tosend.data.move.x, tosend.data.move.y);
    			
    			zaznacz_ruch(&mojaPlansza, aktGracz, tosend.data.move.x+1, tosend.data.move.y+1);
				pokaz_plansze(&mojaPlansza);
				zmien_gracza(&aktGracz);
				ssend = 1;
    		}
    		else
    		{
    			printf("ruch drugiego gracza\n");
    			aktGracz = 1;
    		}
    		break;
    		
    		case MOVE:
    			//TODO zaznaczanie ruchu przeciwnika
    			printf("recieved: x:%hhu y:%hhu\n", recvd.data.move.x, recvd.data.move.y);
    			zaznacz_ruch(&mojaPlansza, aktGracz, recvd.data.move.x+1, recvd.data.move.y+1);
    			//TODO czy int ma znaczenie w zaznacz_ruch
				pokaz_plansze(&mojaPlansza);
				zmien_gracza(&aktGracz);
				
    			tosend.type = (uint8_t)MOVE; tosend.len = (uint8_t)4;
    			printf("Twoj ruch\n");
					printf("\tpodaj nr wiersza (1-3): ");
					scanf("%hhu", &tosend.data.move.x);
					printf("\tpodaj nr kolumny (1-3): ");
					scanf("%hhu", &tosend.data.move.y);
					tosend.data.move.x--;
					tosend.data.move.y--;
    			printf("sending: x:%hhu y:%hhu\n", tosend.data.move.x, tosend.data.move.y);
    			
    			//zaznacz_ruch(&mojaPlansza, aktGracz, tosend.data.move.x, tosend.data.move.y);
    			ssend = 1;
    			
    			zaznacz_ruch(&mojaPlansza, aktGracz, tosend.data.move.x+1, tosend.data.move.y+1);
				pokaz_plansze(&mojaPlansza);
				zmien_gracza(&aktGracz);
    		break;
    		
    		case MESSAGE:
    			printf("%s: %.80s", player2, recvd.data.text);
    		break;
    		
    		case STATE:
    			game_end = 1;
    			if(recvd.data.state == win)
    				printf("Wygrales!\n");
    			else if(recvd.data.state == lose)
    				printf("Przegrales.\n");
    			else printf("Remis.\n");
    		break;
    	}
    	//send tutaj
    	if(ssend)
    	{
    		send(sd, &tosend, sizeof(tosend), 0);
    		ssend = 0;
    	}
    	
    	free(Bufor);
    	
    }
    return 0;
}
