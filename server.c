#include "common.h"

int msgid;
int shmid;
int semid;
pid_t pid;

void handle_sigint(int sig) {
    printf("\n[SERWER] Otrzymano Ctrl+C. Sprzątam...\n");
    if (pid > 0) {
        kill(pid, SIGKILL);
    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1) perror("msgctl remove failed");
    if (shmctl(shmid, IPC_RMID, NULL) == -1) perror("shmctl remove failed");
    if (semctl(semid, 0, IPC_RMID) == -1) perror("semctl remove failed");

    printf("[SERWER] Zasoby zwolnione. Do widzenia!\n");
    exit(0);
}


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
    signal(SIGINT, handle_sigint);

    int unit_costs[4] = {100, 250, 500, 150};

    key_t key = ftok(SERVER, FTOK_ID);
    if (key == -1) { perror("ftok failed"); exit(1); }
    
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) { perror("msgget failed"); exit(1); }
    printf("Kolejka utworzona ID: %d\n", msgid);

    key_t shm_key = ftok(SERVER, FTOK_ID_SHM);
    if (shm_key == -1) { perror("ftok shm failed"); exit(1); }

    shmid = shmget(shm_key, sizeof(GameState), 0666 | IPC_CREAT);
    if (shmid == -1) { perror("shmget failed"); exit(1); }

    GameState *game_state = (GameState *)shmat(shmid, NULL, 0);
    if (game_state == (void *)-1) { perror("shmat failed"); exit(1); }

    game_state->connected_players = 0;
    game_state->resource[0] = 1000;
    game_state->production_timer[0] = 0;
    game_state->production_timer[1] = 0;

    for(int i=0; i<4; i++) game_state->units[0][i] = 0;

    key_t sem_key = ftok(SERVER, FTOK_ID_SEM);
    if (sem_key == -1) { perror("ftok sem failed"); exit(1); }

    semid = semget(sem_key, 1, 0666 | IPC_CREAT);
    if (semid == -1) { perror("semget failed"); exit(1); }

    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) { perror("semctl failed"); exit(1); }

    printf("Serwer gotowy. Ctrl+C aby zakończyć.\n");

    pid = fork();
    
    if (pid == 0) {
        while (1) {
            sleep(1);
            lock(semid);
            if (int i = 0; i<2; i++) {
                game_state->resource[i] += 5;
                // printf("[EKONOMIA] Surowce: %d\n", game_state->resource[0]);
                if (game_state->units_in_queue[i] > 0) {
                    production_timer[i]--;
                    if (production_timer[i] <= 0) {
                        int type = game_state->production_type[i];
                        units[i][type]++;
                        game_state->units_in_queue[i]--;
                        production_timer[i] = 3;
                        printf("[SERWER]: Gracz %d wyprodukował jednostkę typu: %d", i, type);
                    }
                } 
            }
            unlock(semid);
        }
    } else {
        Message msg;
        while(1) {
            if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 1, 0) == -1) {
                perror("msgrcv failed");
                continue;
            }

            switch (msg.type) { 
                case MSG_DATA:
                    lock(semid);
                    msg.data[0] = game_state->resource[0];
                    unlock(semid);
                    
                    msg.mtype = msg.snd_id;
                    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
                    break;

                case MSG_TRAIN:
                    int type = msg.data[0];
                    int client_pid = msg.snd_id;
                    int cost = unit_costs[type];

                    lock(semid);
                    if (game_state->resource[0] >= cost) {
                        game_state->resource[0] -= cost;
                        game_state->units_in_queue++;
                        game_state->production_type = type;
                        game_state->units[0][type]++;

                        
                        sprintf(msg.mtext, "Kupiono jednostke typ %d!", type);
                        printf("[SERWER] Gracz kupil typ %d. Zostalo zlota: %d\n", type, game_state->resource[0]);
                    } else {
                        sprintf(msg.mtext, "Brak zlota! Koszt: %d", cost);
                    }
                    unlock(semid);

                    msg.mtype = client_pid;
                    msg.data[0] = game_state->resource[0];
                    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
                    break;
            }
        }
    }
    return 0;
}