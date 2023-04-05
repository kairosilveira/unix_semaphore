#include "CL_include"

/************************************************
 *VARIAVEIS GLOBAIS***********************
 *****************************/
typedef int SEMAPHORE;
BUF *Memptr;
int shmid;
char cle_acces_mem[200];
int voie;
SEMAPHORE lect;
key_t cle;

int pfd_1[2];
int pfd_2[2];

/************************************************
 *FUNÇÕES***********************
 *****************************/
int getIdMessagerie();
void creerSegment(int size, char *name);

void handler_1(int n);
void handler_2(int n);

SEMAPHORE Creer_sem(key_t key);
void Detruire_sem(SEMAPHORE sem);
void Changer_sem(SEMAPHORE sem, int val, int numero);
void P(SEMAPHORE sem, int num);
void V(SEMAPHORE sem, int num);
key_t Creer_cle(char *nom_fichier);

void lecteur_0();
void lecteur_1();

void redacteur_1(int pfd_1[2]);
void redacteur_2(int pfd_2[2]);

/************************************************
 *PROGRAMA PRINCIPAL***********************
 *****************************/
int main(int argc, char const *argv[])
{
    int id_msg;
    int pid = getpid();
    int pid_lecteur0, pid_lecteur1;
    int pid_redacteur1, pid_redacteur2;
    dmsgbuf message, message_rvd;

    /************************************************
     *Definindo pipeline***********************
     *****************************/
    if (pipe(pfd_1) == -1)
    {
        perror("main : pipe(pfd_1)");
    }

    if (pipe(pfd_2) == -1)
    {
        perror("main : pipe(pfd_2)");
    }

    printf("pid: %d\n", pid);

    /************************************************
     *Connexion client-serveur***********************
     *****************************/

    if ((id_msg = getIdMessagerie()) < 0) /*Obtendo msqid para conectar ao servidor*/
    {
        printf("CL: Erreur getIdMessagerie %d\n", MSGerr);
    }

    message.type = CONNECT;          /*Definindo o tipo da mensagem como CONNECT*/
    sprintf(message.txt, "%d", pid); // transforma o PDI em uma string que é salva em message.txt

    if (msgsnd(id_msg, &message, strlen(message.txt) + 1, 0) < 0) /*Envia mensagem ao servidor usando id_msg e message */
    {
        printf("CL: erreur msgsnd :%d\n", MSGerr);
    }

    /************************************************
     *Recebe uma mensagem do servidor, e salva(CleClient) em message_rvd.txt***********************
     *****************************/
    if (msgrcv(id_msg, &message_rvd, L_MSG, pid, 0) < 0)
    {
        printf("CL:erreur msgrcv :%d\n", MSGerr);
    }

    /************************************************
     *Enviando msg ACK para confirmar o recebimento da chave(CleClient)***********************
     *****************************/

    message.type = ACK;
    sprintf(message.txt, "%d", pid);
    if (msgsnd(id_msg, &message, strlen(message.txt) + 1, 0) < 0)
    {
        printf("CL: erreur msgsnd :%d\n", MSGerr);
    }
    printf("Message reveiced: %s\n", message_rvd.txt);

    cle = Creer_cle(message_rvd.txt);
    strcpy(cle_acces_mem, message_rvd.txt);
    lect = Creer_sem(cle);
    creerSegment(2 * sizeof(BUF), cle_acces_mem);
    printf("cle: %d\n", cle);

    if ((pid_lecteur0 = fork())) // fazer o if de erro para os forks
    {
        /*******criação dos redatores********/
        if ((pid_redacteur1 = fork()) == 0)
        {
            redacteur_1(pfd_1);
            exit(0);
        }

        if ((pid_redacteur2 = fork()) == 0)
        {
            redacteur_2(pfd_2);
            exit(0);
        }

        if ((pid_lecteur1 = fork())) /*le pere*/
        {
            signal(SIGUSR1, handler_1);
            signal(SIGUSR2, handler_2);

            for (int i = 0; i < 50; i++)
            {
                pause();
                if (voie == 0)
                {
                    V(lect, 0);
                }
                else
                {
                    V(lect, 1);
                }
            }
        }
        else /*le premier fils*/
        {
            lecteur_0();
            exit(0);
        }
    }
    else /*la deuxieme fils*/
    {
        lecteur_1();
        exit(0);
    }

    kill(pid_lecteur0, SIGKILL);
    kill(pid_lecteur1, SIGKILL);
    kill(pid_redacteur1, SIGKILL);
    kill(pid_redacteur2, SIGKILL);
    Detruire_sem(lect);

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

/*************************/
/****** creer un cle pour accedre a la memoire*******/
/*************************/

void creerSegment(int size, char *name)
{

    int shmid;  /* l'identificateur de la memoire partagee  */
    key_t clef; /* la clef associee au segment */

    /* L'instruction ftok(name,(key_t) cle) permet de construire
       une cle identifiant le segment */
    clef = ftok(name, C_Shm);
    printf("clef: %d\n", clef);
    shmid = shmget(clef, size, 0);

    if (shmid == -1)
    {
        perror("La creation du segment de memoire partage a echouee");
        exit(1); /* on laisse tout tomber et on sort du programme */
    }

    /*attacher a la segment*/
    Memptr = (BUF *)shmat(shmid, 0, 0);

    printf("Tshmid =  %d \n", shmid);
}

key_t Creer_cle(char *nom_fichier)
{
    key_t cle;

    if ((cle = ftok(nom_fichier, '0')) == -1)
    {
        perror("Creer_cle");
        exit(EXIT_FAILURE);
    }
    return cle;
}

/*****************************************************les fonctions des semaphores ****************************************/
SEMAPHORE Creer_sem(key_t key)
{
    SEMAPHORE sem;
    int r;
    int nombre_de_sem = 2;
    int val_init = 0;
    sem = semget(key, nombre_de_sem, IPC_CREAT | IPC_EXCL | 0666);
    if (sem < 0)
    {
        perror("Creer_sem : semget");
        exit(EXIT_FAILURE);
    }
    r = semctl(sem, nombre_de_sem - 1, SETVAL, val_init);
    if (r < 0)
    {
        perror("Creer_sem : semctl");
        exit(EXIT_FAILURE);
    }

    r = semctl(sem, nombre_de_sem - 2, SETVAL, val_init); /*creation du deuxieme semaphore de la meme famille*/
    if (r < 0)
    {
        perror("Creer_sem : semctl");
        exit(EXIT_FAILURE);
    }
    return sem;
}

/* **************************************** */
/* Destruction d'un ensemble de semaphore   */
/*                                          */
/* On utilise donc semctl avec IPC_RMID     */
/* **************************************** */
void Detruire_sem(SEMAPHORE sem)
{
    int nombre_de_sem = 1;
    if (semctl(sem, nombre_de_sem - 1, IPC_RMID, 0) != 0)
    {
        perror("Detruire_sem");
        exit(EXIT_FAILURE);
    }
}

/* **************************************** */
/* Modification de la valeur des semaphores */
/*                                          */
/* appartenant a un ensemble de semaphores. */
/* On utilise donc semop                    */
/* **************************************** */
void Changer_sem(SEMAPHORE sem, int val, int numero)
{
    struct sembuf sb[1];
    /* int nombre_de_sem=1;*/

    sb[0].sem_num = numero;
    sb[0].sem_op = val;
    sb[0].sem_flg = 0;

    if (semop(sem, sb, 1) != 0) /*j'ai mis 1 car nous on va modifier 1 seul semaphore*/
    {
        perror("Changer_sem");
        exit(EXIT_FAILURE);
    }
}

/* **************************************** */
/* **************************************** */
void P(SEMAPHORE sem, int num)
{
    Changer_sem(sem, -1, num);
}

/* **************************************** */
/* **************************************** */
void V(SEMAPHORE sem, int num)
{
    Changer_sem(sem, 1, num);
}

/*****************************************************les fonctions des signaux ****************************************/

void handler_1(int n)
{
    signal(SIGUSR1, handler_1);
    voie = 0;
}

void handler_2(int n)
{
    signal(SIGUSR2, handler_2);
    voie = 1;
}

void lecteur_0()
{
    int donnee;
    close(pfd_1[0]);

    while (1)
    {
        P(lect, 0);
        donnee = Memptr[0].tampon[Memptr[0].n];
        // donnee = Memptr->tampon[Memptr->n];
        // printf("Donne du voie 1(lecteur) : %d\n", donnee);
        write(pfd_1[1], &donnee, sizeof(donnee));
    }
}
void lecteur_1()
{
    int donnee;
    close(pfd_2[0]);

    while (1)
    {
        P(lect, 1);
        donnee = Memptr[1].tampon[Memptr[1].n];
        // printf("Donne du voie 2 (lecteur): %d\n", donnee);
        write(pfd_2[1], &donnee, sizeof(donnee));
    }
}

/******************Redatores*******************************/
void redacteur_1(int pfd_1[2])
{
    int donnee;
    int i;

    time_t t;
    char *c_time;
    char donnee_traitee[300];
    char inter[12];

    close(pfd_1[1]);
    // close(pfd_driver[0]);

    while (1)
    {
        sprintf(donnee_traitee, "%s", "\n\nBuffer 1:\n");
        for (i = 0; i < 5; i++)
        {
            read(pfd_1[0], &donnee, sizeof(donnee));
            time(&t);
            c_time = ctime(&t);
            sprintf(inter, "%d", donnee);

            strcat(donnee_traitee, "date : ");
            strcat(donnee_traitee, c_time);
            strcat(donnee_traitee, "donnee : ");
            strcat(donnee_traitee, inter);
            strcat(donnee_traitee, "\n");
            // printf("%steste=========\n", donnee_traitee);
        }
        printf("%s", donnee_traitee);
        // write(pfd_driver[1], donnee_traitee, sizeof(donnee_traitee));
    }
}

void redacteur_2(int pfd_2[2])
{
    int donnee;
    int i;

    time_t t;
    char *c_time;
    char donnee_traitee[300];
    char inter[12];

    close(pfd_2[1]);
    // close(pfd_driver[0]);

    while (1)
    {
        sprintf(donnee_traitee, "%s", "\n\nBuffer 2:\n");
        for (i = 0; i < 5; i++)
        {
            read(pfd_2[0], &donnee, sizeof(donnee));
            time(&t);
            c_time = ctime(&t);
            sprintf(inter, "%d", donnee);

            strcat(donnee_traitee, "date : ");
            strcat(donnee_traitee, c_time);
            strcat(donnee_traitee, "donnee : ");
            strcat(donnee_traitee, inter);
            strcat(donnee_traitee, "\n");
            // printf("%steste=========\n", donnee_traitee);
        }
        printf("%s", donnee_traitee);
        // write(pfd_driver[1], donnee_traitee, sizeof(donnee_traitee));
    }
}
// ipcs
// ipcrm -a