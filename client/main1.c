
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

//Serwer domyslny
#define SERVER "127.0.0.1"

/* Server's port number */
#define SERVPORT 2015

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

	
	// BUFOR, TEMP I SENDBUF
    char * Bufor;
    message_t temp;
    message_t sendbuf;

	
	message_t test;
	// OBSLUGA MOJEGO IMIENIA I WYSLANIE JOIN
    char name1[20];
	memset(&sendbuf, 0, sizeof(sendbuf));
	
    printf("Podaj imie: \n");
    scanf("%19s", name1);
	/*
    sendbuf->type=JOIN;
    strcpy(sendbuf->data.name, name1);
    send(sd, sendbuf, sizeof(sendbuf), 0);
	free(sendbuf);
	
	*/
	 test.type=JOIN;
    strcpy(test.data.name, name1);
    send(sd, &test, sizeof(test), 0);
	
	// ZMIENNE PRZYDATNE W GRZE
	char dGracz[20];
	int inGame = 0;
	int nrWiersza;
    int nrKolumny;
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
					char temp1[sizeof(temp)];
					memcpy(temp1, &temp, sizeof(temp));
					for(int i = 0; i < sizeof(temp); ++i) printf("%hhu ", temp1[i]);
					printf("\ntype: %hhu len: %hhu\n", temp.type,temp.len);
					printf("RECIEVE CORRECT!\n");
					break;
				case 2:   // twoj ruch  (SEND MOVE)
					pokaz_plansze(&mojaPlansza);
					choice =0;
					printf("Chcesz wyslac ruch(0) czy wiadomosc(1)?\n");
					scanf("%d", &choice);
					switch(choice){ // WYBIERZ TYP NADANIA 	
						case 0:
							sendbuf.type=MOVE;
							printf("Twoj ruch\n");
							printf("Podaj nr wiersza (1-3): \n");
							int x,y;
							scanf("%d", &x);
							printf("Podaj nr kolumny (1-3): \n");
							scanf("%d", &y);
							if(x-1 < 0 || x-1 > 2 || y-1 < 0 || y-1 > 2 ){
								printf("Podano bledne wspolrzedne\n Sprobuj ponownie.\n");
								break;
							}
							sendbuf.data.move.x = (uint8_t)(x-1);
							sendbuf.data.move.y = (uint8_t)(y-1);
							if (zaznacz_ruch(&mojaPlansza, aktGracz, sendbuf.data.move.x, sendbuf.data.move.y) == 0){
									printf("Podano bledne wspolrzedne\n Sprobuj ponownie\n");
							}
							else{
								send(sd, &sendbuf, sizeof(sendbuf), 0);
								printf("Wyslano\n");
								//printf("Wyslano x == %d\n", sendbuf->data.move.x);
								//printf("Wyslano y == %d\n", sendbuf->data.move.y);
								pokaz_plansze(&mojaPlansza);
								zmien_gracza(&aktGracz);
								zmienna = 1;
								localType = 1;
							}
							break;
						
						case 1:
							printf("Wprowadz wiadomosc: \n");
							memset(&wiado, 0, 80);
							scanf("%79s", wiado);
							printf("%s\n", wiado);
							sendbuf.type = MESSAGE;
							strcpy(sendbuf.data.text, wiado);
							printf("%s\n", sendbuf.data.text);
							send(sd, &sendbuf, sizeof(sendbuf), 0);
							break;
						
						default:
							break;
						
					} 			
				default:
					break;
			}
		if(localType == 1 && zmienna == 0){
			switch(temp.type){   // switch od komunikacji z serwerem
				case JOIN:            // JOIN
					strcpy(dGracz, temp.data.name);
					//memcpy(&dGracz, &temp->data.name, 20);
					printf("TWOJ PRZECIWNIK: %s\n", dGracz);
					inGame = 1;
					break;
				case START:			  // START
					if (temp.data.turn){
						Gracz = G_KOLKO;
						printf("Jestes kolko\n");
						localType = 2;
					}
					else {
						Gracz = G_KRZYZYK;
						printf("Jestes krzyzyk\n");
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
						pokaz_plansze(&mojaPlansza);
						printf("Przegranko!\n");
						printf("GAME OVER !\n");
					}
					return 0;
					break;
				
				case MOVE:            // MOVE
					zaznacz_ruch(&mojaPlansza, aktGracz, temp.data.move.x, temp.data.move.y);
					printf("Przed zmiana gracza!\n");
					zmien_gracza(&aktGracz);
					localType = 2;
					break;
						
				case MESSAGE:		  // MESSAGE
					printf("test\n");
					//printf("Wiadonko od %19s: %79s\n", dGracz, temp->data.text);
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
