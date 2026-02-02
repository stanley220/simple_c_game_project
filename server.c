#include "common.h"

void lock(int semid) {
    struct sembuf bufor;
    bufor.sem_num = 0;
    bufor.sem_op = -1;
    bufor.sem_flg = 0;
    if (semop(semid, &bufor, 1) == -1) {
        perror("semop lock failed");
        exit(EXIT_FAILURE);
    }
}

void unlock(int semid) {
    struct sembuf bufor;
    bufor.sem_num = 0;
    bufor.sem_op = 1;
    bufor.sem_flg = 0;
    if (semop(semid, &bufor, 1) == -1) {
        perror("semop unlock failed");
        exit(EXIT_FAILURE);
    }
}


int main() {

    int unit_costs[4] = {100, 250, 500, 150};

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
    
    //msgctl(msgid, IPC_RMID, NULL);


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

    //shmdt(game_state);
    //shmctl(shmid, IPC_RMID, NULL);
    //semctl(semid, 0, IPC_RMID);

    

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        while (1) {
            sleep(1);
            lock(semid);
            game_state->resource[0] += 10;
            printf("Child: Player 1 resources: %d\n", game_state->resource[0]);
            unlock(semid);
        }
        
    } else {
        while(1) {
            Message msg;
            if (msgrcv(msgid, &msg, sizeof(msg.mtext) + sizeof(int), 0, 0) == -1) {
                perror("msgrcv failed");
                exit(EXIT_FAILURE);
            } else {
                switch (msg.mtype) {
                case MSG_LOGIN:
                    printf("Received login message from %d: %s\n", msg.snd_id, msg.mtext);
                    break;
                case MSG_DATA:
                    lock(semid);
                    msg.data[0] = game_state->resource[0];
                    unlock(semid);
                    msg.mtype = msg.snd_id;
                    msgsnd(msgid, &msg, sizeof(msg.mtext) + sizeof(int), 0);
                    break;
                case MSG_TRAIN:
                    int type = msg.data[0];
                    int client_pid = msg.snd_id;
                    if (type < 0 || type > 3) {
                        strcpy(msg.mtext, "Błąd!");
                    }
                    int cost = unit_costs[type];
                    lock(semid);
                    if (game_state->resource[0] >= cost) {
                        game_state->resource[0] -= cost;
                        game_state->production_queue++;
                        game_state->units[0][type]++;
                        strcpy(msg.mtext, "Kupiono jednostkę typu %d!");
                        printf("[SERWER]: Gracz kupił jednostę %d za %d złota\n", type, cost);
                    } else {
                        strcpy(msg.mtext, "Za mało złota aby przepradzić zakup!");
                    }
                    unlock(semid);

                    msg.mtype = client_pid;
                    msg.data[0]=game_state->resource[0];

                    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
                    break;
                default:
                    printf("Unknown message type received: %ld\n", msg.mtype);
                    break;
            }
        }
        
    }
    return 0;
}
}
