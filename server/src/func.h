//functions
#ifndef FUNC_H
#define FUNC_H
#include "msg.h"
#include "structs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
void *cl_session(void *arg);
void *sender(void *arg);
uint8_t wincheck(game* game);
void reset(game* game);
int ifended();
#endif
