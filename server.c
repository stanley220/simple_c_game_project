#include "common.h"

int msgid;
int shmid;
int semid;
pid_t pid;
int players_pids[2] = {0, 0};

void handle_sigint(int sig) {
    printf("\n[SERWER]: Otrzymano Ctrl+C. Sprzątam...\n");
    if (pid > 0) {
        kill(pid, SIGKILL);
    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1) perror("msgctl remove failed");
    if (shmctl(shmid, IPC_RMID, NULL) == -1) perror("shmctl remove failed");
    if (semctl(semid, 0, IPC_RMID) == -1) perror("semctl remove failed");

    printf("[SERWER]: Zasoby zwolnione. Do widzenia!\n");
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

void battle(GameState *gs, int attacker_id) {
    int defender_id;
    if (attacker_id == 0) {
        defender_id = 1;
    } else {
        defender_id = 0;
    }

    float attack_stats[4] = {1.0, 1.5, 3.5, 0.0};
    float defense_stats[4] = {1.2, 3.0, 1.2, 0.0};

    double S_A = 0;
    double S_B = 0;

    for (int i = 0; i<4; i++) {
        S_A += gs->units[attacker_id][i];
    }

    for (int i = 0; i < 4; i++) {
        S_B += gs->units[defender_id][i];
    }

    printf("[BITWA] Gracz %d atakuje gracza %d!\nSiła armii atakującej: %d\nSiła armii broniącej: %d", attacker_id, defender_id, S_A, S_B);

    if (S_A > S_B) {
        for (int i = 0; i < 4; i++) {
            gs->units[defender_id][i] = 0;
        }
    } else {
        // x*(sa/sb)
        if (S_B > 0) {
            for (int i=0; i<4; i++) {
                int lost = (int)(gs->units[defender_id][i]*(S_A/S_B));
                gs->units[defender_id][i]-=lost;
                if (gs->units[defender_id][i]<0) {
                    gs->units[defender_id][i] = 0;
                }
            }
        }
    }

    if (S_A > 0) {
        for(int i = 0; i<4; i++) {
            int lost = (int)(gs->units[attacker_id][i]*(S_B/S_A));
            gs->units[attacker_id][i] -= lost;
            if(gs->units[attacker_id][i] < 0) {
                gs->units[attacker_id][i] = 0;
            }
        }
    }
}




int main() {
    signal(SIGINT, handle_sigint);

    int unit_costs[4] = {100, 250, 500, 150};

    key_t key = ftok(SERVER, FTOK_ID);
    msgid = msgget(key, 0666 | IPC_CREAT);
    
    key_t shm_key = ftok(SERVER, FTOK_ID_SHM);
    shmid = shmget(shm_key, sizeof(GameState), 0666 | IPC_CREAT);
    GameState *game_state = (GameState *)shmat(shmid, NULL, 0);

    game_state->connected_players = 0;
    game_state->resource[0] = 300;
    game_state->resource[1] = 300;

    for (int i = 0; i<2; i++) {
        game_state->production_timer[i] = 0;
        game_state->units_in_queue[i] = 0;
        for (int j = 0; j<4; j++) {
            game_state->units[i][j] = 0;
        }
    }

    for(int i=0; i<4; i++) game_state->units[0][i] = 0;

    key_t sem_key = ftok(SERVER, FTOK_ID_SEM);
    semid = semget(sem_key, 1, 0666 | IPC_CREAT);
    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    printf("Serwer gotowy. Ctrl+C aby zakończyć.\n");

    pid = fork();
    
    if (pid == 0) {
        while (1) {
            sleep(1);
            lock(semid);
            
            for (int i = 0; i < 2; i++) { 
                int workers = game_state->units[i][3];
                game_state->resource[i] += 50 + (workers*5);
                
                if (game_state->units_in_queue[i] > 0) {
                    
                    game_state->production_timer[i]--;
                    
                    if (game_state->production_timer[i] <= 0) {
                        int type = game_state->production_type[i];
                        
                        game_state->units[i][type]++; 
                        game_state->units_in_queue[i]--;
                        
                        game_state->production_timer[i] = 0; 
                        
                        printf("[SERWER]: Gracz %d wyprodukował jednostkę typu: %d\n", i+1, type+1);
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
                case MSG_LOGIN:
                    lock(semid);
                    int assigned_id = -1;
                    if(players_pids[0]==msg.snd_id) {
                        assigned_id = 0;
                    } else if (players_pids[1]==msg.snd_id) {
                        assigned_id = 1;
                    } else {
                        if (players_pids[0]==0) {
                            players_pids[0] = msg.snd_id;
                            assigned_id = 0;
                        } else if (players_pids[1]==0) {
                            players_pids[1] = msg.snd_id;
                            assigned_id = 1;
                        }
                    }
                    unlock(semid);

                    msg.mtype = msg.snd_id;
                    msg.player_id = assigned_id;
                    if (assigned_id != -1) {
                        strcpy(msg.mtext, "[SERWER]: Witaj w grze!");
                    } else {
                        strcpy(msg.mtext, "[SERWER]: Serwer pełny!");
                    }

                    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
                    printf("[SERWER]: Logowanie PID=%d -> ID=%d\n", msg.snd_id, assigned_id);
                    break;

                case MSG_DATA:
                    int p_id = msg.player_id;
                    lock(semid);
                    if (p_id>-1 && p_id<2) {
                        msg.data[0] = game_state->resource[p_id];
                    }
                    unlock(semid);
                    msg.mtype = msg.snd_id;
                    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
                    break;

                case MSG_TRAIN:
                    int id = msg.player_id;
                    int type = msg.data[0];
                    int client_pid = msg.snd_id;
                    
                    if (type < 0 || type > 3) break; 
                    
                    int cost = unit_costs[type];

                    lock(semid);
                    if (id>-1 && id <2) {
                        if (game_state->units_in_queue[id] > 0) {
                            strcpy(msg.mtext, "[SERWER]: Kolejka zajeta!");
                        }
                        else if (game_state->resource[id] >= cost) {
                            game_state->resource[id] -= cost;
                        
                            game_state->units_in_queue[id]++;
                            game_state->production_type[id] = type;
                            game_state->production_timer[id] = 3;

                            strcpy(msg.mtext, "[SERWER]: Budowa rozpoczęta...");
                            printf("[SERWER] Gracz kupil typ %d. Zostalo zlota: %d\n", type+1, game_state->resource[0]);
                        } else {
                            strcpy(msg.mtext, "[SERWER]: Nie wystarczjąca ilość surowców!");
                        }
                    }
                    unlock(semid);

                    msg.mtype = client_pid;
                    if (id>-1 && id <2) {
                        msg.data[0] = game_state->resource[id];
                    }
                    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
                    break;
                
                case MSG_ATTACK:
                    int atc_id = msg.player_id;
                    lock(semid);
                    battle(game_state, atc_id);
                    if (game_state->victory_points[atc_id]>=5) {
                        strcpy(msg.mtext, "[SERWER]: KONIEC GRY!");
                    } else {
                        strcpy(msg.mtext, "[SERWER]: Atak zakończony.");
                    }
                    unlock(semid);
                    msg.mtype = msg.snd_id;
                    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
                    break;
            }
        }
    }
    return 0;
}