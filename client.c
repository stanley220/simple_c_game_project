#include "common.h"

int main() {
    key_t key = ftok(SERVER, FTOK_ID);
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Nie moge polaczyc sie z kolejka (Uruchom najpierw serwer!)");
        exit(1);
    }

    pid_t pid = getpid();
    int my_id = -1;
    Message msg;

    printf("Łączenie z serwerem...\n");
    msg.mtype = MSG_LOGIN;
    msg.snd_id = pid;
    msg.type = MSG_LOGIN;
    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
    msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);
    my_id = msg.player_id;

    if (my_id == -1) {
        printf("Wystąpił błąd podczas logowania: %s\n", msg.mtext);
        exit(1);
    }

    printf("Zalogowano jako - GRACZ %d\n", my_id+1);
    printf("Wiadomosc: %s\n", msg.mtext);

    int option;
    while(1) {
        printf("\n[GRACZ %d] MENU:\n1. Sprawdź ekwipunek\n2. Kup jednostki\n3. Atak\nWybierz: ", my_id+1);
        if (scanf("%d", &option) != 1) {
            while(getchar() != '\n');
            continue;
        }

        msg.mtype = 1;
        msg.snd_id = pid;
        msg.player_id = my_id;

        if (option == 1) {
            msg.type = MSG_DATA;
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);
            printf("Twoje surowce: %d\n", msg.data[0]);
        } 
        else if (option == 2) {
            int unit_type;
            printf("Wybierz typ (1-4):\n1. Lekka (100)\n2. Cieżka (250)\n3. Jazda (500)\n4. Robotnicy (150)\nWybor: ");
            scanf("%d", &unit_type);

            if (unit_type < 1 || unit_type > 4) {
                printf("Niepoprawny typ!\n");
                continue;
            }
            msg.type = MSG_TRAIN;
            msg.data[0] = unit_type - 1;
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);

            printf("Serwer: %s\n", msg.mtext);
            printf("Surowce po zakupie: %d\n", msg.data[0]);
        }
        else if (option == 3 ) {
            msg.mtype = 1;
            msg.snd_id = pid;
            msg.type = MSG_ATTACK;
            msg.player_id = my_id;

            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);
            
            printf("Raport bitewny: %s\n", msg.mtext);
        }
    }
    return 0;
}