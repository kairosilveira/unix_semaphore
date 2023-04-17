#include "CL_include"

/**************************************
**********VARIABLES GLOBALES**********
**************************************/

/*memoire partage*/
BUF *Memptr;
int shmid;
char cle_acces_mem[200];
int voie;

/*Semaphore*/
typedef int SEMAPHORE;
SEMAPHORE lect;
SEMAPHORE sem_driver;
key_t cle;
key_t cle_driver;

/*pipe*/
int pfd_1[2];
int pfd_2[2];
int pfd_driver[2];

/*driver*/
char buf[100];
int nwritten;

/*************************************
**************FONCTIONS***************
*************************************/
void handle_alarm(int sig);

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

void lecteur_1();
void lecteur_2();

void redacteur_1(int pfd_1[2], int pfd_driver[2]);
void redacteur_2(int pfd_2[2], int pfd_driver[2]);

/*************************************
*********PROGRAMME PRINCIPAL**********
*************************************/
int main(int argc, char const *argv[])
{
    int id_msg;
    int pid = getpid();
    int pid_lecteur1, pid_lecteur2;
    int pid_redacteur1, pid_redacteur2;
    int pid_driver;
    int time_exe = 25;
    dmsgbuf message, message_rvd;

    /***************************************
     **DÉFINITION DE LA DURÉE DU PROGRAMME***
     ***************************************/
    if (argc > 1)
    {
        time_exe = atoi(argv[1]);
    }

    printf("*******************************************\n");
    printf("Projet CLIENT - SERVEUR Unix IESE 4\n\n");
    printf("Auteurs : MACHADO William ET SILVEIRA Kairo\n\n");
    printf("*******************************************\n\n");
    printf("!!! Lancement client (pendant %d s) !!!\n\n", time_exe);
    printf("Pid du client = %d\n\n", pid);
    printf("*****************connecte******************\n");

    signal(SIGALRM, handle_alarm);
    alarm(time_exe);

    /**************************************
    ********DÉFINITION DES PIPELINES********
    **************************************/
    if (pipe(pfd_1) == -1)
    {
        perror("CL : Erreur_Creer_pipe(pfd_1)");
    }

    if (pipe(pfd_2) == -1)
    {
        perror("CL : Erreur_Creer_pipe(pfd_2)");
    }

    if (pipe(pfd_driver) == -1)
    {
        perror("CL : Erreur_Creer_pipe(pfd_2)");
    }

    /**************************************
    *******CONNEXION CLIENT-SERVEUR*******
    **************************************/
    
    if ((id_msg = getIdMessagerie()) < 0) // Obtenir msqid pour se connecter au serveur
    {
        printf("CL: Erreur getIdMessagerie %d\n", MSGerr);
    }

    message.type = CONNECT;                                       // Définition du type de message comme CONNECT
    sprintf(message.txt, "%d", pid);                              // transforme le PID en une chaîne qui est enregistrée dans message.txt
    if (msgsnd(id_msg, &message, strlen(message.txt) + 1, 0) < 0) // Envoyer un message au serveur en utilisant id_msg et message
    {
        printf("CL: Erreur msgsnd :%d\n", MSGerr);
    }

    /**********************************************************************************
    **REÇOIT UN MESSAGE DU SERVEUR, ET L'ENREGISTRE (CleClient) DANS message_rvd.txt**
    **********************************************************************************/
    if (msgrcv(id_msg, &message_rvd, L_MSG, pid, 0) < 0)
    {
        printf("CL: Erreur msgrcv :%d\n", MSGerr);
    }

    /**************************************************************
    **ENVOI DU MESSAGE ACK POUR CONFIRMER LA RÉCEPTION DE LA CLÉ(CleClient)**
    **************************************************************/
    message.type = ACK;
    sprintf(message.txt, "%d", pid);
    if (msgsnd(id_msg, &message, strlen(message.txt) + 1, 0) < 0)
    {
        printf("CL: Erreur msgsnd :%d\n", MSGerr);
    }
    // printf("Message reveiced: %s\n", message_rvd.txt);

    /**************************************
     ******LA CREATION DES SEMAPHORES******
     **************************************/
    cle = Creer_cle(message_rvd.txt);
    cle_driver = Creer_cle("CL.c");

    lect = Creer_sem(cle);
    sem_driver = Creer_sem(cle_driver);

    /*********************************************
     **LA CREATION DE SEGMENT DE MEMOIRE PARTAGE***
     *********************************************/
    strcpy(cle_acces_mem, message_rvd.txt);
    creerSegment(2 * sizeof(BUF), cle_acces_mem);
    // printf("cle: %d\n", cle);

    /**************************************
    ****************DRIVER****************
    **************************************/
    if ((pid_driver = fork()) == -1)
    {
        perror("CL: Erreur_Creer_driver");
        exit(EXIT_FAILURE);
    }
    else if (pid_driver == 0)
    {
        // Processus fils : exécute le programme "./drive"
        close(pfd_driver[1]); // Ferme l'extrémité d'écriture du pipe

        // Rediriger l'entrée standard vers l'extrémité de lecture du pipe
        if (dup2(pfd_driver[0], STDIN_FILENO) == -1)
        {
            perror("CL: Erreur_DUP2");
            exit(EXIT_FAILURE);
        }

        // Ferme l'extrémité de lecture du pipe car l'entrée standard pointe maintenant vers elle
        close(pfd_driver[0]);

        // Exécute le programme "./drive"
        execlp("./drive", "./drive", (char *)NULL);

        // Si l'exécution arrive ici, une erreur s'est produite lors de l'exécution de "./drive"
        perror("CL: Erreur_EXEC");
        exit(EXIT_FAILURE);
    }

    /*************************************
    **************REDACTEUR***************
    *************************************/
    if ( (pid_lecteur1 = fork()) == -1 )
    {
      perror("CL: Erreur_Creer_Lecteur_1");
      exit(EXIT_FAILURE);
    } 
    else if (pid_lecteur1 != 0)
    {
        if ((pid_redacteur1 = fork()) == -1)
        {
             perror("CL: Erreur_Creer_Redacteur_1");
             exit(EXIT_FAILURE);
        } 
        else if (pid_redacteur1 == 0)
        {
            redacteur_1(pfd_1, pfd_driver);
            exit(0);
        }

        if ((pid_redacteur2 = fork()) == -1)
        {
             perror("CL: Erreur_Creer_Redacteur_2");
             exit(EXIT_FAILURE);
        } 
        else if (pid_redacteur2 == 0)
        {
            redacteur_2(pfd_2, pfd_driver);
            exit(0);
        }

        /*************************************
        ***************LECTEUR****************
        *************************************/

        if ((pid_lecteur2 = fork()) == -1)
        {
             perror("CL: Erreur_Creer_Lecteur_2");
             exit(EXIT_FAILURE);
        } 
        else if (pid_lecteur2 != 0)
        {
            signal(SIGUSR1, handler_1);
            signal(SIGUSR2, handler_2);

            for (int i = 0; i < time_exe; i++)
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
            lecteur_1();
            exit(0);
        }
    }
    else /*le deuxieme fils*/
    {
        lecteur_2();
        exit(0);
    }

    kill(pid_lecteur1, SIGKILL);
    kill(pid_lecteur2, SIGKILL);
    kill(pid_redacteur1, SIGKILL);
    kill(pid_redacteur2, SIGKILL);
    kill(pid_driver, SIGKILL);
    Detruire_sem(lect);
    Detruire_sem(sem_driver);

    printf("****************deconnecte*****************\n");
    return 0;
}

/******************************************************
**FUNCTION POUR GERER LE TEMPS D'EXECUTION DU CLIENT**
******************************************************/
void handle_alarm(int sig)
{
    printf("Fin d'execution du Client!!!\n");
    exit(0);
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

/********************************************************
**FUNCTION POUR CREER UN CLE POUR ACCEDRE A LA MEMOIRE**
********************************************************/
void creerSegment(int size, char *name)
{

    int shmid;  /* l'identificateur de la memoire partagee  */
    key_t clef; /* la clef associee au segment */

    /* L'instruction ftok(name,(key_t) cle) permet de construire
       une cle identifiant le segment */
    clef = ftok(name, C_Shm);
    //printf("clef : %d\n", clef);
    shmid = shmget(clef, size, 0);

    if (shmid == -1)
    {
        perror("La creation du segment de memoire partage a echouee");
        exit(1); /* on laisse tout tomber et on sort du programme */
    }

    /*attacher a la segment*/
    Memptr = (BUF *)shmat(shmid, 0, 0);

    printf("ptr tampon : %d \n", shmid);
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

/**************************************
*****LES FONCTIONS DES SEMAPHORES*****
**************************************/
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

/*************************************
******LES FONCTIONS DES SIGNAUX*******
*************************************/
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

/*************************************
*****LES FONCTIONS DES LECTEURS ******
*************************************/
void lecteur_1()
{
    int donnee;
    close(pfd_1[0]);

    while (1)
    {
        P(lect, 0);
        donnee = Memptr[0].tampon[Memptr[0].n];
        // donnee = Memptr->tampon[Memptr->n];
        write(pfd_1[1], &donnee, sizeof(donnee));
    }
}
void lecteur_2()
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

/*************************************
*****LES FONCTIONS DES REDATEURS******
*************************************/
void redacteur_1(int pfd_1[2], int pfd_driver[2])
{
    int donnee;
    int i;

    time_t t;
    char *c_time;
    char donnee_traitee[10000];
    char inter_1[12];
    char inter_2[12];
    donnee_traitee[0] = 0;

    close(pfd_1[1]);

    while (1)
    {
        for (i = 0; i < 5; i++)
        {
            read(pfd_1[0], &donnee, sizeof(donnee));
            time(&t);
            c_time = ctime(&t);
            sprintf(inter_1, "%d", donnee);
            sprintf(inter_2, "%d", i);

            strcat(donnee_traitee, "Donnee no : ");
            strcat(donnee_traitee, inter_2);
            strcat(donnee_traitee, " de la voie 2 : ");
            strcat(donnee_traitee, inter_1);          
            strcat(donnee_traitee, " le ");
            strcat(donnee_traitee, c_time);
        }
            strcat(donnee_traitee, "\n");

        // printf("%s", donnee_traitee);
        V(sem_driver, 0);
        close(pfd_driver[0]);
        write(pfd_driver[1], donnee_traitee, strlen(donnee_traitee) + 1);
        P(sem_driver, 0);
        strcpy(donnee_traitee, "");
    }
}

void redacteur_2(int pfd_2[2], int pfd_driver[2])
{
    int donnee;
    int i;

    time_t t;
    char *c_time;
    char donnee_traitee[10000];
    donnee_traitee[0] = 0;
    char inter_1[12];
    char inter_2[12];

    close(pfd_2[1]);

    while (1)
    {
        for (i = 0; i < 5; i++)
        {
            read(pfd_2[0], &donnee, sizeof(donnee));
            time(&t);
            c_time = ctime(&t);
            sprintf(inter_1, "%d", donnee);
            sprintf(inter_2, "%d", i);

            strcat(donnee_traitee, "Donnee no : ");
            strcat(donnee_traitee, inter_2);
            strcat(donnee_traitee, " de la voie 1 : ");
            strcat(donnee_traitee, inter_1);          
            strcat(donnee_traitee, " le ");
            strcat(donnee_traitee, c_time);
        }
        
        strcat(donnee_traitee, "\n");

        V(sem_driver, 0);
        close(pfd_driver[0]);
        write(pfd_driver[1], donnee_traitee, strlen(donnee_traitee) + 1);
        P(sem_driver, 0);
        strcpy(donnee_traitee, "");
    }
}