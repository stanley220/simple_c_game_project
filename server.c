#include "common.h"

int main() {

    key_t key = ftok("server.c", 'A');
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
    return 0;
}
