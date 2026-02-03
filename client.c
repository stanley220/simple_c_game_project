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

    char last_server_msg[100] = "Witaj w grze!";

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

        system("clear");

        msg.mtype = 1;
        msg.snd_id = pid;
        msg.player_id = my_id;
        msg.type = MSG_DATA;
        msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
        msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);

        printf("=========================================\n");
        printf(" GRACZ %d  |  PUNKTY ZWYCIĘSTWA: %d / 5\n", my_id + 1, msg.data[5]);
        printf("=========================================\n");
        printf(" ZŁOTO: %d\n", msg.data[0]);
        printf(" ARMIA:\n");
        printf("  [1] Lekka Piechota:  %d\n", msg.data[1]);
        printf("  [2] Ciężka Piechota: %d\n", msg.data[2]);
        printf("  [3] Jazda:           %d\n", msg.data[3]);
        printf("  [4] Robotnicy:       %d\n", msg.data[4]);
        printf("=========================================\n");

        printf(" OSTATNI KOMUNIKAT: %s\n", last_server_msg);
        printf("=========================================\n");
        
        printf("AKCJE:\n1. Odśwież (Pobierz stan)\n2. Kup jednostkę\n3. Atakuj\n");
        printf("Twój wybór: ");
        if (scanf("%d", &option) != 1) {
            while(getchar() != '\n');
            continue;
        }

        if (option == 1) {
            strcpy(last_server_msg, "Zaktualizowano dane.");
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
            msg.snd_id = pid;
            msg.player_id = my_id;
            msg.type = MSG_TRAIN;
            msg.data[0] = unit_type - 1;
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);

            strcpy(last_server_msg, msg.mtext);
        }
        else if (option == 3 ) {
            msg.mtype = 1;
            msg.snd_id = pid;
            msg.type = MSG_ATTACK;
            msg.player_id = my_id;

            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), pid, 0);
            
            // POPRAWKA: Zapisz wynik do bufora, zamiast wypisywać od razu
            strcpy(last_server_msg, msg.mtext); 
        }
    }
    return 0;
}