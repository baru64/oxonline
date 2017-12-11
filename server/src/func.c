#include "func.h"

void *cl_session(void* arg)
{
	char *buf;
  int msgLen;
  int conIdx = *(int *)(arg);
  printf("cl session id:%d\n", conIdx);
	while(connections[conIdx].finished == 0)
	{
		buf = malloc(1500);

        memset(buf, 0, 1500);
        msgLen = recv(connections[conIdx].fd, buf, 1500, 0);
        message_t temp;
        memset(&temp, 0, sizeof(temp));
        memcpy(&temp, buf, msgLen);
        
        pthread_mutex_lock(&mutex); //zamykamy semafor
        
        IPCbuffer.player[IPCbuffer.writeIdx] = conIdx;
        IPCbuffer.messages[IPCbuffer.writeIdx] = temp;
        printf("Msg added to buffer. from %d %d\n", conIdx, IPCbuffer.player[IPCbuffer.writeIdx]);
        
        IPCbuffer.writeIdx++;
        if (IPCbuffer.writeIdx == BUF_LEN)
            IPCbuffer.writeIdx = 0;
            
       	pthread_mutex_unlock(&mutex); //otwieramy semafor
        
    }
    
    free(buf);


	close(connections[conIdx].fd);
    connections[conIdx].finished = 1;
    return NULL;
}

void *sender(void *arg)
{
	while(1)
    {
        if (IPCbuffer.readIdx != IPCbuffer.writeIdx)
        {
            //printf("Ipc buffer sending\n");
            message_t *msg = &IPCbuffer.messages[IPCbuffer.readIdx];

 			printf("Received msg type: %hhu\n", msg->type);
            switch(msg->type)
            {
            	case JOIN:
            		printf("join recieved. name = %s\n",msg->data.name);
            		
            		memcpy(GAME.player_name[IPCbuffer.player[IPCbuffer.readIdx]], msg->data.name, 20);
            		if(connections[0].notEmpty == 1 && connections[1].notEmpty == 1) //czy obaj gracze sa polaczeni
            		{
            			//wyslanie obu graczom joina i start(losowanie kto zaczyna), zapisanie odpowiedniego gracza w active_player
            			message_t join;
            			join.type = JOIN; join.len = 22;
            			memcpy(join.data.name,GAME.player_name[0], 20);
            			printf("name: %s \n",GAME.player_name[0]);
            			send(connections[1].fd, &join, join.len, 0);
            			memcpy(join.data.name,GAME.player_name[1], 20);
            			printf("name: %s \n",GAME.player_name[1]);
            			send(connections[0].fd, &join, join.len, 0);
            			int starting = rand() % 2; //losowanie zaczynajacego
            			GAME.active_player = starting;
            			message_t start;
            			start.type = START; start.len = 6;
            			start.data.turn = true;
            			send(connections[starting].fd, &start, start.len, 0);
            			printf("sending:%hhu turn:%d\n", start.type, start.data.turn);
            			starting = (starting + 1) % 2;
            			start.data.turn = false;
            			send(connections[starting].fd, &start, start.len, 0);
            			printf("join forwarded, starting player: %d\n", starting);
            		}
            	break;
            	case MOVE:
            		printf("Move recieved. x=%hhu y=%hhu len=%hhu sizeof=%lu\n", msg->data.move.x,msg->data.move.y,msg->len,sizeof(msg)); //TODO sprawdz czy wysyla sie do drugiego gracza
            		char temp1[sizeof(msg)];
            		memcpy(temp1, msg, sizeof(msg));
            		for(int i = 0; i < sizeof(msg); ++i) printf("%hhu ", temp1[i]);
            		if( (GAME.board[msg->data.move.x+msg->data.move.y*3] == '-')
            		 && (IPCbuffer.player[IPCbuffer.readIdx] == GAME.active_player)
            		 && (connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].notEmpty == 1) )
            		//czy ruch jest poprawny, czy pochodzi od poprawnego gracza(active player) i czy drugi gracz jest polaczony
            		{
            			printf("Recieved correct move\n");
            			if(GAME.active_player)
            				GAME.board[msg->data.move.x+msg->data.move.y*3] = 'x';
            			else
            				GAME.board[msg->data.move.x+msg->data.move.y*3] = 'o';
            			
            			int temp_if = 0;
            			if(ifended()) temp_if = 1;
            			send(connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].fd, msg, 8, 0);
            			printf("Move: x=%hhu y=%hhu len=%hhu\n", msg->data.move.x, msg->data.move.y, msg->len);
            			printf("Move forwarded\n");
            			GAME.active_player = (GAME.active_player + 1) % 2;
            			if(temp_if) //TODO zrobic to ladniej pozniejs
            			{
            				connections[0].finished = 1;
            				connections[1].finished = 1;
            			}
            			//dopisujemy do planszy i wysylamy do drugiego gracza, ustawiamy active_player na drugiego gracza

            		}
            		else //jesli nie to wysylamy refuse do gracza
            		{
            			message_t ref; ref.type = REFUSE; ref.len = 2;
            			send(connections[IPCbuffer.player[IPCbuffer.readIdx]].fd, &ref, ref.len, 0);
            			printf("Move refused\n");
            		}
            	break;
            	case MESSAGE:
            		if(connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].notEmpty == 1) //sprawdzenie czy drugi gracz jest polaczony
            		{
            			send(connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].fd, msg, msg->len, 0);
            			printf("Message forwarded\n");
            		}
            	break;
            }
            IPCbuffer.readIdx++;
            IPCbuffer.readIdx %= BUF_LEN;
            printf("Read id:%d\n", IPCbuffer.readIdx);
            
        }
    }
}

uint8_t wincheck(game* game)
{
  //Sprawdzanie wierszy
  for (int y=0; y<3; y++)
  {
    if ((game->board[3*y] == game->board[3*y + 1]) && (game->board[3*y] == game->board[3*y + 2]) && (game->board[3*y]!= '-'))
    {
      if (game->board[3*y] == 'o')
        game->winner = O;
      if (game->board[3*y] == 'x')
        game->winner = X;
      printf("wincheck1\n");
      return 1;
    }
  }
  //Sprawdzanie kolumn
  for (int x=0; x<3; x++)
  {
    if ((game->board[x] == game->board[x + 3]) && (game->board[x] == game->board[x + 6]) && (game->board[x]!= '-'))
    {
      if (game->board[x] == 'o')
        game->winner = O;
      if (game->board[x] == 'x')
        game->winner = X;
      printf("wincheck2\n");
      return 1;
    }
  }
  //Sprawdzanie przekątnych
  if ((game->board[0] == game->board[4]) && (game->board[0] == game->board[8]) && (game->board[0]!= '-'))
  {
    if (game->board[4] == 'o')
      game->winner = O;
    if (game->board[4] == 'x')
      game->winner = X;
    printf("wincheck3\n");
    return 1;
  }
  if ((game->board[2] == game->board[4]) && (game->board[2] == game->board[6]) && (game->board[2]!= '-'))
  {
    if (game->board[4] == 'o')
      game->winner = O;
    if (game->board[4] == 'x')
      game->winner = X;
    printf("wincheck4\n");
    return 1;
  }
  //sprawdzanie możliwości wykonania ruchu
  char t = 0;
  for (int i=0; i<9; i++)
  {
  	if(game->board[i] == '-')
			++t;
	}
	if(t == 0)
	{
		game->winner = DRAW;
		return 1;
	}
  return 0;
}

void reset(game* game)
{
	for(int i = 0; i < 9; ++i) game->board[i] = '-';
	game->winner = none;
}

int ifended()
{
	if(wincheck(&GAME))
	{
			message_t msg1;
    	   	msg1.type = STATE; msg1.len = 6;
    	  	msg1.data.state = win;
     	 	message_t msg2;
     	 	msg2.type = STATE; msg2.len = 6;
     	 	msg2.data.state = lose;
     	 	msg1.len = (uint8_t)sizeof(msg1);
     	 	msg2.len = (uint8_t)sizeof(msg2);
    		printf("Game finished, sending state.winner=%d\n", GAME.winner);
   			switch(GAME.winner)
       		{	
       			case O:
       			send(connections[0].fd, &msg1, msg1.len, 0);
       			//connections[0].finished = 1;
       			send(connections[1].fd, &msg2, msg2.len, 0);
       			//connections[1].finished = 1;
       			printf("O won.\n");
       			break;
       			
       			case X:
       			send(connections[0].fd, &msg2, msg2.len, 0);
       			//connections[0].finished = 1;
       			send(connections[1].fd, &msg1, msg1.len, 0);
       			//connections[1].finished = 1;
       			printf("X won.\n");
       			break;
       		
       			case DRAW: ;
       			message_t msg;
       			msg.type = STATE; msg.len = 6;
       			msg.data.state = draw;
       			send(connections[0].fd, &msg, msg.len, 0);
       			//connections[0].finished = 1;
       			send(connections[1].fd, &msg, msg.len, 0);
       			//connections[1].finished = 1;
       			printf("Draw.\n");
       			break;
       			case none:
       			printf("Something is no yes XD\n");
       		}
       		
       		reset(&GAME);
       		return 1;
	}
	return 0;
}
