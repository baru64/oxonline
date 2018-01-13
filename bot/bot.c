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
#include <time.h>
#include "msg.h"

//Serwer domyslny
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
    //nrWiersza--;
    //nrKolumny--;
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

_Bool wygrana(plansza_t *plansza, ruch_t g)
{
_Bool test=false;
int i;
	for(i = 1; i <= 7; i += 3) test |= ((plansza->pole[i] == g) && (plansza->pole[i+1] == g) && (plansza->pole[i+2] == g));// wiersze
	for(i = 1; i <= 3; i += 1) test |= ((plansza->pole[i] == g) && (plansza->pole[i+3] == g) && (plansza->pole[i+6] == g));// kolumny
	test |= ((plansza->pole[1] == g) && (plansza->pole[5] == g) && (plansza->pole[9] == g));// przekatne
	test |= ((plansza->pole[3] == g) && (plansza->pole[5] == g) && (plansza->pole[7] == g));
	
	return test;
}

_Bool remis(plansza_t *plansza)
{
for(int i = 1; i <= 9; i++) if(plansza->pole[i] == BRAK) return false;
else return true;
}

int minimax(plansza_t *plansza, gracz_t g)
{
  ruch_t gracz;
  if (g == G_KRZYZYK)gracz = KRZYZYK;
  if (g == G_KOLKO)gracz = KOLKO;
  int m, mmx;
  
// Najpierw sprawdzamy, czy bieżący gracz wygrywa na planszy. Jeśli tak, to
// zwracamy jego maksymalny wynik

  if(wygrana(plansza,gracz)) return (gracz == KRZYZYK) ? 1 : -1;

// Następnie sprawdzamy, czy nie ma remisu. Jeśli jest, zwracamy wynik 0

  if(remis(plansza)) return 0;

// Będziemy analizować możliwe posunięcia przeciwnika. Zmieniamy zatem
// bieżącego gracza na jego przeciwnika

  gracz = (gracz == KRZYZYK) ? KOLKO : KRZYZYK;

// Algorytm MINIMAX w kolejnych wywołaniach rekurencyjnych naprzemiennie analizuje
// grę gracza oraz jego przeciwnika. Dla gracza oblicza maksimum wyniku gry, a dla
// przeciwnika oblicza minimum. Wartość mmx ustawiamy w zależności od tego, czyje
// ruchy analizujemy:
// X - liczymy max, zatem mmx <- -10
// O - liczymy min, zatem mmx <-  10

  mmx = (gracz == KOLKO) ? 10 : -10;

// Przeglądamy planszę szukając wolnych pół na ruch gracza. Na wolnym polu ustawiamy
// literkę gracza i wyznaczamy wartość tego ruchu rekurencyjnym wywołaniem
// algorytmu MINIMAX. Planszę przywracamy i w zależności kto gra:
// X - wyznaczamy maximum
// O - wyznaczamy minimum
int i;
  for(i = 1; i <= 9; i++)
    if(plansza->pole[i] == BRAK)
    {
       plansza->pole[i] = gracz;
       m = minimax(plansza,gracz);
       plansza->pole[i] = BRAK;
       if(((gracz == KOLKO) && (m < mmx)) || ((gracz == KRZYZYK) && (m > mmx))) mmx = m;
    }
  return mmx;
}

int bot(plansza_t *plansza)
{
  int ruch, i, m=0, mmx;

  for(i=0; i<9; i++)
  {
	if(plansza->pole[i] == BRAK)
    	{
	  m++;
	}
  }
	if(m==9)
	{
	  srand(time(NULL));
	  printf("Pusta plansza");
	  return rand()%9;
	}

  mmx = -10;
  for(i = 1; i <= 9; i++)
    if(plansza->pole[i] == BRAK)
    {
      plansza->pole[i] = KRZYZYK;
      m = minimax(plansza,KRZYZYK);
      plansza->pole[i] = KOLKO;
      if(m > mmx)
      {
        mmx = m; ruch = i;     
      }        
    }    
  return ruch;
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
        printf("Utworzono gniazdo bota\n");

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
            printf("---This is a bot program---\n");
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


	// BUFOR, TEMP I SENDBUF
    char * Bufor;
    message_t temp;
    message_t sendbuf;


	message_t test;
	// OBSLUGA MOJEGO IMIENIA I WYSLANIE JOIN
    	char name1[20];
	srand(time(NULL));
	memset(&sendbuf, 0, sizeof(sendbuf));
	char imiona[10][20] = {"Wolfgang", "Calineczka", "Myszon", "Janusz", "Karyna", "Napoleon", "Belzebub", "Nestor", "Brajan", "Guantanamera"};
	strcpy(name1, imiona[rand()%10]);

	test.type=JOIN;
    	strcpy(test.data.name, name1);
    	send(sd, &test, sizeof(test), 0);

	// ZMIENNE PRZYDATNE W GRZE
	char dGracz[20];
	int inGame = 0;
	int nrWiersza;
    int nrKolumny;
	int polozenie;
    plansza_t mojaPlansza;
    czysc_plansze(&mojaPlansza);
    gracz_t aktGracz = G_KOLKO;
    gracz_t Gracz;
	int localType = 1;
	int zmienna = 0;
	int choice =0;
	char wiado[80];
	// START WHILE
	while(1){
			Bufor = (char*)malloc(1500);
		   	memset(Bufor, 0, 1500);
			memset( &temp, 0, sizeof(temp) );
			memset( &sendbuf, 0, sizeof(sendbuf) );
			zmienna = 0;
			switch(localType){ 		// zarzadzanie nasza gra
				case 1:   // zwykly odbior z serwera
					printf("Czekanko na serwerek!\n");
					msgLen = recv(sd, Bufor, 1500, 0);
					memcpy(&temp, Bufor, msgLen);
					//char temp1[sizeof(temp)];
					//memcpy(temp1, &temp, sizeof(temp));
					//for(int i = 0; i < sizeof(temp); ++i) printf("%hhu ", temp1[i]);
					//printf("\ntype: %hhu len: %hhu\n", temp.type,temp.len);
					printf("RECIEVE CORRECT!\n");
					break;
				case 2:   // twoj ruch  (SEND MOVE)
					pokaz_plansze(&mojaPlansza);
					sendbuf.type=MOVE;
					polozenie = bot(&mojaPlansza);
					printf("Moj ruch:%d\n", polozenie);
					nrWiersza = polozenie/3;
					nrKolumny = polozenie%3;
					printf("Kolumna: %d Wiersz: %d\n", nrKolumny,nrWiersza);
					sendbuf.data.move.x = (uint8_t)(nrWiersza);
					sendbuf.data.move.y = (uint8_t)(nrKolumny);
					zaznacz_ruch(&mojaPlansza, aktGracz, sendbuf.data.move.x, sendbuf.data.move.y);
					send(sd, &sendbuf, sizeof(sendbuf), 0);
					printf("Wyslano ruch\n");
					zmien_gracza(&aktGracz);
					zmienna = 1;
					localType = 1;
					break;
					
				default:
					break;
			}
		if(localType == 1 && zmienna == 0){
			switch(temp.type){   // switch od komunikacji z serwerem
				case JOIN:            // JOIN
					strcpy(dGracz, temp.data.name);
					printf("MOJ PRZECIWNIK: %s\n", dGracz);
					inGame = 1;
					break;
				case START:			  // START
					if (temp.data.turn){
						Gracz = G_KOLKO;
						localType = 2;
						printf("Jestem kolko\n");
					}
					else {
						Gracz = G_KRZYZYK;
						printf("Jestem krzyzyk\n");						
					}
					break;

				case REFUSE:          // REFUSE
					// REFUSE NA JOIN
					if(!inGame){
					perror("Serwer zajety!\n");
					close(sd);
					exit(-1);
					}
					else{
					// REFUSE NA MOVE
						printf("ZLE WYSLANY MOVE!!\n");
					}
					break;

				case STATE: 			 // STATE
					 printf("Koniec gry: \n");
					if(temp.data.state==win){
						printf("Wygranko!\n");
						printf("GAME OVER !\n");
					}
					else if(temp.data.state==draw){
						free(Bufor);
						Bufor = (char*)malloc(1500);
						memset(Bufor, 0, 1500);
						memset( &temp, 0, sizeof(temp) );	
						msgLen = recv(sd, Bufor, 1500, 0);
						memcpy(&temp, Bufor, msgLen);
						zaznacz_ruch(&mojaPlansza, aktGracz, temp.data.move.x, temp.data.move.y);
						printf("Remisik\n");
						printf("GAME OVER !\n");
					}
					else{ // PRZEGRANA - jeszcze dostaniesz MOVE przeciwnika
						free(Bufor);
						Bufor = (char*)malloc(1500);
						memset(Bufor, 0, 1500);
						memset( &temp, 0, sizeof(temp) );	
						msgLen = recv(sd, Bufor, 1500, 0);
						memcpy(&temp, Bufor, msgLen);
						zaznacz_ruch(&mojaPlansza, aktGracz, temp.data.move.x, temp.data.move.y);
						//pokaz_plansze(&mojaPlansza);
						printf("Przegranko!\n");
						printf("GAME OVER !\n");
					}
					return 0;
					break;

				case MOVE:            // MOVE
					zaznacz_ruch(&mojaPlansza, aktGracz, temp.data.move.x, temp.data.move.y);
					//printf("Przed zmiana gracza!\n");
					zmien_gracza(&aktGracz);
					localType = 2;
					break;

				case MESSAGE:		  // MESSAGE
					printf("test\n");
					printf("Wiadomonko od %s: %s\n", dGracz, temp.data.text);
					break;

					default:
						printf("RECIEVE CORRECT IN SWITCH BUT NONE TYPE OR BROKEN\n");
						break;
			}
		}
		free(Bufor);
	}
	// KONIEC WHILE
    return 0;
}
