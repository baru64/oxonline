#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "structs.h"
#include "msg.h"
#include "func.h"

int main()
{
	printf("%s\n", test_text);
	return 0;
}
