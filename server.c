#include "common.h"

int main() {

    key_t key = ftok("server.c", FTOK_ID);
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    printf("Message queue created with ID: %d\n", msgid);

    Message msg;
    
    msgrcv(msgid, &msg, sizeof(msg.mtext) + sizeof(int), MSG_LOGIN, 0);
    printf("Received message from client %d: %s\n", msg.snd_id, msg.mtext);

    msgctl(msgid, IPC_RMID, NULL);


    key_t shm_key = ftok("server.c", FTOK_ID_SHM);
    if (shm_key == -1) {
        perror("ftok for shm failed");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(shm_key, sizeof(GameState), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    GameState *game_state = (GameState *)shmat(shmid, NULL, 0);
    if (game_state == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }


    return 0;
}
