//wiadomosc przekazywana miedzy klientem a serwerem
#define HDR_SIZE 2
typedef enum
{
  JOIN,			//pole name wewnatrz
  REFUSE,		//nic wewnatrz
  MOVE,			//pole move
  START,		//pole turn
  STATE,		//pole game_state
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
	} data;

} message_t __attribute__ ((packed));
