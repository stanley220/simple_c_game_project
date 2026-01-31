#include "common.h"

int main() {
    key_t key = ftok("server.c", FTOK_ID);
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    Message msg;

    msg.mtype = MSG_LOGIN;
    strcpy(msg.mtext, "Witam!");

    msgsnd(msgid, &msg, sizeof(msg.mtext) + sizeof(int), 0);
}

