#include "common.h"

int main() {
    key_t key = ftok(SERVER, FTOK_ID);
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    
    int option;
    while(1) {
        printf("Menu:\n1. Odśwież\n2. Kup jednostkę\nWybierz opcję: ");
        scanf("%d", &option);

        if(option == 1) {
            Message msg;
            msg.mtype = MSG_DATA;
            msg.snd_id = pid;
            strcpy(msg.mtext, "Wniosek o podanie zasobów");
            msgsnd(msgid, &msg, sizeof(msg.mtext) + sizeof(int), 0);
            msgrcv(msgid, &msg, sizeof(msg.mtext) + sizeof(int), pid, 0);
            printf("Złoto: %d\n", msg.data[0]);
        } else if (option == 2) {
            Message msg;
            msg.mtype = MSG_TRAIN;
            msg.snd_id = pid;
            printf("Wybierz typ jednostki (1-4): \n1: Lekka piechota - 100\n2: Ciężka piechota - 250\n3: Jazda - 550\n4: Robotnicy - 100\n");
            int unit_type;
            printf("Numer typu jednostki: ");
            scanf("%d", &unit_type);
            msg.data[0] = unit_type-1;
            strcpy(msg.mtext, "Wniosek o kupienie jednostki");
            msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
            msgrcv(msgid, &msg, sizeof(msg.mtext) + sizeof(int), pid, 0);
            printf("[SERWER]: Kupiono jednostkę typu %d", unit_type);
            printf("Złoto: %d\n", msg.data[0]);
        }
    }

    //    int msgid = msgget(key, 0666);
    //    if (msgid == -1) {
    //        perror("msgget failed");
    //        exit(EXIT_FAILURE);
    //  }
    //  Message msg;

    //  msg.mtype = MSG_LOGIN;
    //  strcpy(msg.mtext, "Witam!");

    //  msgsnd(msgid, &msg, sizeof(msg.mtext) + sizeof(int), 0);
        


}

