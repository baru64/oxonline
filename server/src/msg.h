//wiadomosc przekazywana miedzy klientem a serwerem
#include <stdint.h>
#define HDR_SIZE 2
#ifndef MSG_H
#define MSG_H
typedef enum
{
  JOIN,			//pole name wewnatrz
  REFUSE,		//nic wewnatrz
  MOVE,			//pole move
  START,		//pole turn
  STATE,		//pole game_state
  MESSAGE;		//wiadomosc - pole text
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
		
		char text[80];
	} data;

} message_t __attribute__ ((packed));
#endif
