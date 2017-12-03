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
    do
    {
        memset(buf, 0, 1500);
        msgLen = recv(connections[conIdx].fd, buf, 1500, 0);
        message_t temp;
        memcpy(&temp, buf, msgLen);
        IPCbuffer.player[IPCbuffer.writeIdx] = conIdx;
        IPCbuffer.messages[IPCbuffer.writeIdx] = temp;
        printf("Msg added to buffer\n");
        
        IPCbuffer.writeIdx++;
        if (IPCbuffer.writeIdx == BUF_LEN)
            IPCbuffer.writeIdx = 0;
            
       	
        
    }
    
    free(buf);


	}
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
            printf("Ipc buffer sending\n");
            message_t *msg = &IPCbuffer.messages[IPCbuffer.readIdx];
            																//TODO ----V----
            	//ifended wywala graczy - clsession() bedzie sie zamykal gdy finished
            	//dodac semafor do clsess
            	// #przyejrzystosckodu
 
            switch(msg->type)
            {
            	case JOIN:
            		printf("join recieved. name = %s\n",msg->data.name);
            		//GAME.player_name[IPCbuffer.player[IPCbuffer.readIdx]] = msg->data.name;
            		memcpy(GAME.player_name[IPCbuffer.player[IPCbuffer.readIdx]], msg, 20);
            		if(connections[0].notEmpty == 1 && connections[1].notEmpty == 1) //czy obaj gracze sa polaczeni
            		{
            			//wyslanie obu graczom joina i start(losowanie kto zaczyna), zapisanie odpowiedniego gracza w active_player
            			message_t join;// = {JOIN, 22, GAME.player_name[1]};
            			join.type = JOIN; join.len = 22;
            			memcpy(join.data.name,GAME.player_name[0], 20);
            			send(connections[1].fd, &join, join.len, 0);
            			memcpy(join.data.name,GAME.player_name[1], 20);
            			send(connections[0].fd, &join, join.len, 0);
            			int starting = rand() % 2; //losowanie zaczynajacego
            			GAME.active_player = starting;
            			message_t start;
            			start.type = START; start.len = 6;
            			start.data.turn = true;
            			send(connections[starting].fd, &start, start.len, 0);
            			starting = (starting + 1) % 2;
            			start.data.turn = false;
            			send(connections[starting].fd, &start, start.len, 0);
            		}
            	break;
            	case MOVE:
            		if( (GAME.board[msg->data.move.x+msg->data.move.y*3] == '-') && (IPCbuffer.player[IPCbuffer.readIdx] == GAME.active_player) && (connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].notEmpty == 1) )
            		//czy ruch jest poprawny, czy pochodzi od poprawnego gracza(active player) i czy drugi gracz jest polaczony
            		{
            			printf("Recieved correct move\n");
            			GAME.active_player ? GAME.board[msg->data.move.x+msg->data.move.y*3] == 'x' : GAME.board[msg->data.move.x+msg->data.move.y*3] == 'o';
            			send(connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].fd, msg, msg->len, 0);
            			GAME.active_player = (GAME.active_player + 1) % 2;
            			//dopisujemy do planszy i wysylamy do drugiego gracza, ustawiamy active_player na drugiego gracza
            			ifended();
            		}
            		else //jesli nie to wysylamy refuse do gracza
            		{
            			message_t ref; ref.type = REFUSE; ref.len = 2;
            			send(connections[(IPCbuffer.player[IPCbuffer.readIdx]+1)%2].fd, ref, ref.len, 0);
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
            
            
        }
    }
}

uint8_t wincheck(game* game)
{
  //Sprawdzanie wierszy
  for (int y=0; y<3; y++)
  {
    if ((game->board[3*y] == game->board[3*y + 1]) && (game->board[3*y] == game->board[3*y + 2]))
    {
      if (game->board[3*y] == 'o')
        game->winner = O;
      if (game->board[3*y] == 'x')
        game->winner = X;
      return 1;
    }
  }
  //Sprawdzanie kolumn
  for (int x=0; x<3; x++)
  {
    if ((game->board[x] == game->board[x + 3]) && (game->board[x] == game->board[x + 6]))
    {
      if (game->board[x] == 'o')
        game->winner = O;
      if (game->board[x] == 'x')
        game->winner = X;
      return 1;
    }
  }
  //Sprawdzanie przekątnych
  if ((game->board[0] == game->board[4]) && (game->board[0] == game->board[8]))
  {
    if (game->board[4] == 'o')
      game->winner = O;
    if (game->board[4] == 'x')
      game->winner = X;
    return 1;
  }
  if ((game->board[2] == game->board[4]) && (game->board[2] == game->board[6]))
  {
    if (game->board[4] == 'o')
      game->winner = O;
    if (game->board[4] == 'x')
      game->winner = X;
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

int ifended() //sprawdzic poprawnosc TODO
{
	if(wincheck(&GAME))
     	   {
				message_t msg1;
    	   		msg1.type = STATE;
    	   		msg1.data.state = win;
     	  		message_t msg2;
     	 		msg2.type = STATE;
     		 	msg2.data.state = lose;
       		
       			switch(GAME.winner)
       			{	
       				case O:
       				
       				IPCbuffer.player[IPCbuffer.writeIdx] = 0;
        			IPCbuffer.messages[IPCbuffer.writeIdx] = msg1;
        			
        			IPCbuffer.writeIdx++;
       				if (IPCbuffer.writeIdx == BUF_LEN)
       		 		  IPCbuffer.writeIdx = 0;
         		  
        			IPCbuffer.player[IPCbuffer.writeIdx] = 1;
        			IPCbuffer.messages[IPCbuffer.writeIdx] = msg2;
        			
        			IPCbuffer.writeIdx++;
       				if (IPCbuffer.writeIdx == BUF_LEN)
        	 		  IPCbuffer.writeIdx = 0;
       				break;
       			
       				case X:
       			
       				IPCbuffer.player[IPCbuffer.writeIdx] = 0;
       		 		IPCbuffer.messages[IPCbuffer.writeIdx] = msg2;
       	 		
       		 		IPCbuffer.writeIdx++;
       				if (IPCbuffer.writeIdx == BUF_LEN)
       	  		  IPCbuffer.writeIdx = 0;
         		  
       		 		IPCbuffer.player[IPCbuffer.writeIdx] = 1;
       		 		IPCbuffer.messages[IPCbuffer.writeIdx] = msg1;
       	 		
       		 		IPCbuffer.writeIdx++;
       				if (IPCbuffer.writeIdx == BUF_LEN)
       	  		  IPCbuffer.writeIdx = 0;
       				break;
       			
       				case DRAW: ;
       				message_t msg;
       				msg.type = STATE;
       				msg.data.state = draw;
       				IPCbuffer.player[IPCbuffer.writeIdx] = 0;
       		 		IPCbuffer.messages[IPCbuffer.writeIdx] = msg;
        		
        			IPCbuffer.writeIdx++;
       				if (IPCbuffer.writeIdx == BUF_LEN)
         			  IPCbuffer.writeIdx = 0;
         		  
        			IPCbuffer.player[IPCbuffer.writeIdx] = 1;
        			IPCbuffer.messages[IPCbuffer.writeIdx] = msg;
        			
        			IPCbuffer.writeIdx++;
       				if (IPCbuffer.writeIdx == BUF_LEN)
         			  IPCbuffer.writeIdx = 0;
       				break;
       		}
       		
       		reset(&GAME);
       		while(IPCbuffer.readIdx != IPCbuffer.writeIdx);
					close(connections[conIdx].fd);
					connections[conIdx].finished = 1;
       	}
}
