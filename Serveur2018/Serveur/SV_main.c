/********************************************************
*	Module SV_main.c				*
*-------------------------------------------------------*
*	Taches du serveur				*
*	- Allocation du buffer des PID clients		*
*	- Dialogue par message avec les clients		*
*	- Generation des donnees et information clients	*
* 	- Gestion du Mutex				*
********************************************************/
#include "SV_include"
#include "SV_msg.h"
#include "SV_mem.h"
#include "SV_init.h"
#include "SV_data.h"
#include "SV_connect.h"
#include "SV_sem.h"
#include <signal.h>

/* fonction pour traiter les signaux de fin et finir de mani�re propre. */
static void end();
static int PereID,EnregID,RepondID;
static int Tshmid,Cshmid,Msqid, Semid;

int main(int argc, char *argv[])	/********** MAIN */
{
  BUF	*Tptr;
  int     *Cptr;
 
  int	Voie,nbdata;
  long    donnee[6];
  
  signal(SIGTERM, end);
  signal(SIGINT, end);
  
  if (argc < 2)
    {
      /* printf("Usage : sv_zz  <nombre de donnees>"); */
      printf("Erreur dans le passage des parametres pour sv_zz\n");
      exit(0);
    }
  
  nbdata=atoi(argv[1]);
  if (nbdata <= 0)
    {
      printf("Erreur dans la valeur du parametre de sv_zz\n");
      exit(0);
    }
  
  /*#	INITIALISATION RESSOURCES PARTAGEES   #*/
  PereID=getpid();
  donnee[0]= 0;      /* Creation d'un fichier vide */
  printf("\nSrv:Creation Fichiers de cle %d \n",GereFichierCle(donnee));
  Msqid = CreationMessagerie();
  if ((Semid = CreationMutex()) == -1)
    {
      printf("\nSrv:ERREUR fork :RelMesg %d\n",RelacheMessagerie(Msqid));
      exit(0);
    }
  if (Init_Mutex(Semid, 1) == SEMerr)	/* Init du semaphore Mutex a 1 */
    {
      printf("\nSrv:ERREUR fork :RelMesg %d\n",RelacheMessagerie(Msqid));
      printf("Srv:Destruction Mutex %d\n",DestructionMutex(Semid));
      exit(0);
    }
  Cshmid = AllocMemoireClient(&Cptr);
  *Cptr=-1;                              /* premier emplacement libre */
  Tshmid = AllocTampon(&Tptr);
  Tptr->n= -1; (Tptr+1)->n=-1;   /* pointe en arriere de l'indice 0 */
  donnee[0]= time(NULL)+ nbdata; /* temps de fermeture du serveur */
  donnee[1]=Msqid; donnee[2]=Cshmid; /* Id messagerie & mem.Clients */
  donnee[3]= Tshmid; donnee[4]=(long) Tptr; /* Tampons Id & Adresse */
  donnee[5] = Semid;
  GereFichierCle(donnee);	    /* ecrit les donnees dans le fichier CleServeur */
  printf("Srv:Shmid= %d Pointeur PID clients= 0x%x\n",Cshmid, (unsigned) Cptr);
  printf("\nSrv:ID messagerie Msqid = %d\n",Msqid);
  printf("Srv:Tshmid= %d  Pointeur Tampon= 0x%x\n",Tshmid,(unsigned) Tptr);
  printf("Srv:Mutex= %d\n",Semid);
  
  if ( ( EnregID = fork()) == -1 )	/*# 	FILS RECEPTIONNISTE #*/
    {
      GereFichierCle(NULL);
      printf("\nSrv:ERREUR fork :RelMesg %d\n",RelacheMessagerie(Msqid));
      printf("Srv:RelMem %d\n",RelacheMemoires(Tshmid,Cshmid));
      printf("Srv:Destruction Mutex %d\n",DestructionMutex(Semid));
      exit(0);
    }
  if (EnregID == 0 )
    {
      ReceptionClients(Msqid,Cptr);
      printf("Srv:Erreur Reception\n");
      exit(0);
    }
  
  if ( ( RepondID = fork()) == -1 )	/*#	FILS INFORMATEUR	#*/
    {
      GereFichierCle(NULL);
      printf("\nSrv:ERREUR fork :RelMesg %d\n",RelacheMessagerie(Msqid));
      printf("Srv:RelMem %d\n",RelacheMemoires(Tshmid,Cshmid));
      printf("Srv:Destruction Mutex %d\n",DestructionMutex(Semid));
      fflush(stdout);
      exit(0);
    }
  
  if (RepondID == 0 )
    {
      signal(SIGALRM,Timeout);
      alarm(1);
      nbdata = 0;
      while (nbdata < atoi(argv[1]))
	{
          pause();
	  /* la voie est maintenant choisie ici. */
	  Voie = rand()%2;
	  /* et on fait les op�rations sur le s�maphore de la voie */
	  P(Semid, Voie);
	  /* on g�n�re les donn�es */
          GenereData(Tptr, Voie);
	  /* et on lib�re le s�maphore. */
	  V(Semid, Voie);
          nbdata ++;
          SignaleDonnee(++Voie,Cptr);
	}
      fflush(stdout);
      exit(0);
    }
  
  wait(0);
  printf("\nSrv:FIN Del.Fich.Cle %d\n",GereFichierCle(NULL));
  kill(EnregID,SIGKILL);
  printf("\nSrv:FIN Mort du fils receptionniste \n");
  kill(RepondID,SIGKILL);
  printf("Srv:FIN Mort du fils informateur \n");
  printf("\nSrv:FIN RelachMsg %d\n", RelacheMessagerie(Msqid));
  printf("Srv:FIN RelachMem %d\n", RelacheMemoires(Tshmid,Cshmid));
  printf("Srv:FIN Mutex %d\n", DestructionMutex(Semid));
  
  return 0;
}/* mort du pere !!!! */

static void end() { 
  printf("\nSrv:FIN Del.Fich.Cle %d\n",GereFichierCle(NULL));
  printf("\nSrv:FIN Mort du fils receptionniste \n");
  printf("Srv:FIN Mort du fils informateur \n");
  printf("\nSrv:FIN RelachMsg %d\n", RelacheMessagerie(Msqid));
  printf("Srv:FIN RelachMem %d\n", RelacheMemoires(Tshmid,Cshmid));
  printf("Srv:FIN Mutex %d\n", DestructionMutex(Semid));
  kill(0,SIGKILL); /* on tue tout le monde */
}
