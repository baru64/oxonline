#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "structs.h"

#include "func.h"

int main()
{
	memset(&IPCbuffer, 0, sizeof(struct buffer));
	memset(connections, 0, sizeof(connections));
	memset(&GAME, 0, sizeof(struct game));
	reset(&GAME);
	int fdListen;
	
	if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
	
  int port = 2000;
  struct sockaddr_in si_local;
  if ((fdListen=socket(AF_INET, SOCK_STREAM, 0))==-1)
  {
    perror("socket");
    exit(EXIT_FAILURE);
  }

   si_local.sin_family = AF_INET;
   si_local.sin_port  = htons(port);
   memset(&si_local.sin_addr, 0, sizeof(struct in_addr));
   if (bind(fdListen, (const struct sockaddr *)&si_local, sizeof(si_local))==-1)
   {
      perror("bind");
      exit(EXIT_FAILURE);
   }
   listen(fdListen, 2);


   pthread_t sender_t;
   if (pthread_create(&sender_t, NULL, sender, NULL) != 0)
   {
      perror("Can't create sender process\n");
      exit(EXIT_FAILURE);
   }

  while(1)
  {
    int newFd=accept(fdListen, NULL, NULL);

    int i;
    for(i=0; i<LIMIT; i++)
    {
      if (connections[i].finished)
      {
         pthread_join(connections[i].process, NULL);
         connections[i].finished = 0;
         connections[i].notEmpty = 0;
      }

      if (connections[i].notEmpty == 0)
      {
      	 int* temp = (int*) malloc(4);
      	 *temp = i;
         if (pthread_create(&connections[i].process, NULL, cl_session,  temp) != 0)
         {
            perror("Can't create new thread");
            send(newFd, "Can't create new thread for connection handle", 48, 0);
            close(newFd);
         }
         else
         {
         	printf("New connection established");
            connections[i].fd = newFd;
            connections[i].notEmpty = 1;
         }
         break;
      }
    }
    if (i == LIMIT)
    {
    	message_t temp;
    	temp.type = REFUSE;
    	temp.len = 2;
      send(newFd, &temp, temp.len, 0);
      close(newFd);
    }
  }
  return 0;
}
