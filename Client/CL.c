#include "CL_include"
// se leu, seu cu é meu

int getIdMessagerie();

int main(int argc, char const *argv[])
{

    int id_msg;
    int pid = getpid();
    dmsgbuf message, message_rvd;

    printf("pid: %d\n", pid);
    if ((id_msg = getIdMessagerie()) < 0)
    {
        printf("CL: Erreur getIdMessagerie %d\n", MSGerr);
    }

    message.type = CONNECT;
    sprintf(message.txt, "%d", pid);

    if (msgsnd(id_msg, &message, strlen(message.txt) + 1, 0) < 0)
    {
        printf("CL: erreur msgsnd :%d\n", MSGerr);
    }

    if (msgrcv(id_msg, &message_rvd, L_MSG, pid, 0) < 0)
    {
        printf("CL:erreur msgrcv :%d\n", MSGerr);
    }

    printf("Message reveiced: %s\n", message_rvd.txt);
    return 0;
}

int getIdMessagerie()
{
    key_t key;
    int msqid;
    if ((key = ftok(CleServeur, 'M')) < 0)
        return CLEerr;
    msqid = msgget(key, 0666);
    return msqid;
}