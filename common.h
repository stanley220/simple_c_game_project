#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define FTOK_PATH "."
#define FTOK_ID 'P'
#define FTOK_ID_SHM 'S'
#define FTOK_ID_SEM 'E'

#define MSG_LOGIN 1

typedef struct {
    long mtype;     
    int snd_id;   
    char mtext[100];  
} Message;

typedef struct{
    int resource[2];
    int units[2][4];
    int connected_players;
} GameState;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array
};


#endif