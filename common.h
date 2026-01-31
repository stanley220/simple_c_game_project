#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define FTOK_PATH "."
#define FTOK_ID 'P'

#define MSG_LOGIN 1

typedef struct {
    long mtype;     
    int snd_id;   
    char mtext[100];  
} Message;

typedef struct{
    int resource[2];
    int units[2][4];
} GameState;


#endif