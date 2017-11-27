#include <stdio.h>
#include <string.h>

#include "msg.h"
int main()
{
	message_t msg;
	message_t msg2;                             //zapisywanie msg do bufora i z powrotem
	msg.type = JOIN;
	msg.len = 22;
	char nick[] = "craq47qqqqaa1234";
	memcpy(msg.data.name, &nick,sizeof(nick));
	char array[sizeof(msg)];                    //bufor
	memcpy(array, &msg,sizeof(msg));            //do bufora
	printf("%lu\n",sizeof(array));              //lu - long unsigned
	memcpy(&msg2, array, sizeof(array));        //z bufora do struktury
	printf("%hhu %hhu %s",msg2.type,msg2.len,msg2.data.name);
	return 0;                                   //hhu - uint8
}
