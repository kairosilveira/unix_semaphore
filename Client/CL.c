#include "CL_include"
// se leu, seu cu é meu

int getIdMessagerie();

int main(int argc, char const *argv[])
{

    int id_msg;
    int pid = getpid();
    dmsgbuf message, message_rvd;
    message.type = CONNECT;
    id_msg = getIdMessagerie();
    sprintf(message.txt, "%d", pid);
    msgsnd(id_msg, &message, strlen(message.txt) + 1, 0);
    msgrcv(id_msg, &message_rvd, L_MSG, pid, 0);
    printf("Message reveiced: %s\n", message_rvd.txt);
    return 0;
}

int getIdMessagerie()
{
    key_t key;
    int msqid;
    if ((key = ftok(CleServeur, 'M')) < 0)
        return CLEerr;
    msqid = msgget(key, 0666 | IPC_CREAT);
    return msqid;
}