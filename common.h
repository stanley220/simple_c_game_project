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
#include <signal.h>

#define SERVER "server.c"
#define FTOK_PATH "."
#define FTOK_ID 'P'
#define FTOK_ID_SHM 'S'
#define FTOK_ID_SEM 'E'

#define MSG_LOGIN 1
#define MSG_DATA 2
#define MSG_TRAIN 3

typedef struct {
    long mtype;     
    int snd_id;
    int type;
    int data[4];   
    char mtext[100];  
} Message;

typedef struct{
    int resource[2];
    int units[2][4];
    int connected_players;
    int production_timer[2];
    int units_in_queue[2];
    int production_type[2];
} GameState;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


#endif