#include "common.h"

int main() {

    key_t key = ftok(SERVER, FTOK_ID);
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


    key_t shm_key = ftok(SERVER, FTOK_ID_SHM);
    if (shm_key == -1) {
        perror("ftok for shm failed");
        exit(EXIT_FAILURE);
    }
    int shmid = shmget(shm_key, sizeof(GameState), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory created with ID: %d\n", shmid);
    GameState *game_state = (GameState *)shmat(shmid, NULL, 0);
    if (game_state == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    game_state->connected_players = 0;
    game_state->resource[0] = 100;
    printf("Initialized game state in shared memory.\n");
    printf("Player 1 resources: %d\n", game_state->resource[0]);

    key_t sem_key = ftok(SERVER, FTOK_ID_SEM);
    if (sem_key == -1) {
        perror("ftok for sem failed");
        exit(EXIT_FAILURE);
    }
    int semid = semget(sem_key, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    printf("Semaphore created with ID: %d\n", semid);
    union semun arg;
    arg.val = 1;

    if (semctl(semid, 0, SETVAL, arg) == -1){
        perror("semctl failed");
        exit(EXIT_FAILURE);
    };

    shmdt(game_state);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);



    return 0;
}
