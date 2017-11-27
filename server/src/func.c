#include "func.h"

void *cl_session(void* arg)
{
	char *buf;
  int msgLen;
  int conIdx = *(int *)(arg);
	while(GAME.winner == none)
	{
    buf = malloc(1500);
    do
    {
        memset(buf, 0, 1500);
        msgLen = recv(connections[conIdx].fd, buf, 1500, 0);
        message_t temp;
        memcpy(&temp, buf, msgLen);
       
        switch(temp.type)
        {
        	case JOIN:
        		printf("join recieved. name = %s\n",temp.data.name);
        		while(connections[(conIdx+1)%2].notEmpty == 0); // wait for 2nd conn
        		IPCbuffer.player[IPCbuffer.writeIdx] = (conIdx+1)%2;
        		IPCbuffer.messages[IPCbuffer.writeIdx] = temp;
        		printf("join sent to %i\n", (conIdx+1)%2);
        		// send start to opposite player
        		IPCbuffer.writeIdx++;
        		if (IPCbuffer.writeIdx == BUF_LEN)
            	IPCbuffer.writeIdx = 0;
            	
           	message_t start_msg;
           	start_msg.type = START;
           	start_msg.len = 2 + 4;
           	start_msg.data.turn = (conIdx+1)%2; 
        		IPCbuffer.player[IPCbuffer.writeIdx] = (conIdx+1)%2;
        		IPCbuffer.messages[IPCbuffer.writeIdx] = start_msg;
        	break;
        	case MOVE:
        		//przeslij move do drugiego gracza
        		printf("move recieved %i %i\n",temp.data.move.x,temp.data.move.y);
        		IPCbuffer.player[IPCbuffer.writeIdx] = (conIdx+1)%2;
        		IPCbuffer.messages[IPCbuffer.writeIdx] = temp;
        		//jesli ruch powoduje wygrana: wyslij obu graczom status i zamknij polaczenia i thready
        		if(conIdx)
        			GAME.board[temp.data.move.x+temp.data.move.y*3] = 'x';
        		else
        			GAME.board[temp.data.move.x+temp.data.move.y*3] = 'o';
        }
        
        IPCbuffer.writeIdx++;
        if (IPCbuffer.writeIdx == BUF_LEN)
            IPCbuffer.writeIdx = 0;
            
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
       			/*message_t msg1;
       			msg1.type = STATE;
       			msg1.data.state = win;
       			message_t msg2;
       			msg2.type = STATE;
       			msg2.data.state = lose;*/
       			
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
    while (1);
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
            
            if(connections[IPCbuffer.player[IPCbuffer.readIdx]].notEmpty)
            {
              send(connections[IPCbuffer.player[IPCbuffer.readIdx]].fd, msg, msg->len, 0);
              printf("Ipc sent type: %hhu", msg->type);
            }
            
            IPCbuffer.readIdx++;
            IPCbuffer.readIdx %= BUF_LEN;
            
            //free(&IPCbuffer.messages[IPCbuffer.readIdx]); <-wysypuje sie tu
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
