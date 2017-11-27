#include <func.h>
#include <structs.h>
void *cl_session(void* arg)
{
	char *buf;
  int msgLen;
  int conIdx = *(int *)(arg);
	while(GAME->winner == none)
	{
    buf = malloc(1500);
    do
    {
        memset(buf, 0, 1500);
        msgLen = recv(connections[conIdx].fd, buf, 1500, 0);
        //IPCbuffer.messages[IPCbuffer.writeIdx] = buf;
        message_t temp = message_t(buf);
        switch(temp.type)
        {
        	case JOIN:
        		printf("join recieved\n");
        		while(connections[(conIdx+1)%2].notEmpty == 0); // wait for 2nd conn
        		IPCbuffer.player[IPCbuffer.writeIdx] = (conIdx+1)%2;
        		IPCbuffer.messages[IPCbuffer.writeIdx] = temp;
        		printf("join sent to %i\n", (conIdx+1)%2);
        	break;
        	case MOVE:
        		//przeslij move do drugiego gracza
        		printf("move recieved %i %i\n",temp.x,temp.y);
        		IPCbuffer.player[IPCbuffer.writeIdx] = (conIdx+1)%2;
        		IPCbuffer.messages[IPCbuffer.writeIdx] = temp;
        		//jesli ruch powoduje wygrana: wyslij obu graczom status i zamknij polaczenia i thready
        		conIdx ? GAME.board[temp.x+temp.y*3] = 'x' : GAME.board[temp.x+temp.y*3] = 'o';
        }
        
        IPCbuffer.writeIdx++;
        if (IPCbuffer.writeIdx == BUF_LEN)
            IPCbuffer.writeIdx = 0;
            
       	if(wincheck(GAME))
        {
       		message_t endmsg1; //zrob dwa od razu i wysylamy do obydwoch i kasujemy thready
       		message_t endmsg2;
       		endmsg.type = STATE;
       		
       		switch(game->winner)
       		{	
       			case O:
       			conIdx == 0 ? endmsg.data.sate = win : endmsg.data.sate = lose;
       			break;
       			case X:
       			       			conIdx == 0 ? endmsg.data.sate = win : endmsg.data.sate = lose;
       			break;
       			case draw:
       			       			conIdx == 0 ? endmsg.data.sate = win : endmsg.data.sate = lose;
       			break;
       		}
       		//
       	}
        //printf("New message received: %s\n", buf);
        //TODO rozesłać do innych połączeń.
        
    }
    while (msgLen > 0);
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
            //printf("Ipc buffer resending\n");
            struct message_t *msg = &IPCbuffer.messages[IPCbuffer.readIdx];
            
            if(connections[IPCbuffer.player[IPCbuffer.readIdx]].notEmpty)
            {
              send(connections[IPCbuffer.player[IPCbuffer.readIdx]].fd, msg, msg->len, 0);
            }
            
            IPCbuffer.readIdx++;
            IPCbuffer.readIdx %= BUF_LEN;

            //int clId;
            /*for(clId=0; clId<LIMIT; clId++)
            {
                if (connections[clId].notEmpty == 0)
                    continue;

                if (clId == msg->cliId)
                    continue;
                send(connections[clId].fd, msg->msg, msg->length, 0);
            }*/
            
            free(msg);
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
  	if (game->board[i] == '-')
			++t;
	if(t == 0)
	{
		game->winner = draw;
		return 1;
	}
  return 0;
}

void reset(game* game)
{
	for(int i = 0; i < 9; ++i) game->board[i] == '-';
	game->winner = none;
}
