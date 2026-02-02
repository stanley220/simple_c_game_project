#include "common.h"

int main() {
    key_t key = ftok(SERVER, FTOK_ID);
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Nie moge polaczyc sie z kolejka (Uruchom najpierw serwer!)");
        exit(1);
    }

    pid_t pid = getpid();
    int option;
    Message msg;

    printf("Klient uruchomiony (PID: %d)\n", pid);

    while(1) {
        printf("\nMENU:\n1. Odswiez zloto\n2. Kup jednostke\nWybierz: ");
        if (scanf("%d", &option) != 1) {
            while(getchar() != '\n');
            continue;
        }

        if (option == 1) {
            msg.mtype = 1;  
            msg.type = MSG_DATA;   
            msg.snd_id = pid;

            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);
            
            printf(">>> TWOJE ZLOTO: %d <<<\n", msg.data[0]);
        } 
        else if (option == 2) {
            int unit_type;
            printf("Wybierz typ (1-4):\n1. Lekka (100)\n2. Cieżka (250)\n3. Jazda (500)\n4. Robotnicy (150)\nWybor: ");
            scanf("%d", &unit_type);

            if (unit_type < 1 || unit_type > 4) {
                printf("Niepoprawny typ!\n");
                continue;
            }

            msg.mtype = 1;         
            msg.type = MSG_TRAIN;   
            msg.snd_id = pid;
            msg.data[0] = unit_type - 1;

            // Wyślij
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);

            // Odbierz potwierdzenie
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);

            printf("Kupiono jednostkę!\n");
            printf("Zloto po zakupie: %d\n", msg.data[0]);
        }
    }
    return 0;
}