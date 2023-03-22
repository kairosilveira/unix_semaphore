/********************************************************
*	Module SV.c				*
*-----------------------------------------------------------------------*
* Teste  SI parametre (nbdata)  absent ou nul 	      	  		*
*		SINON							*
*	Teste l'existence du fichier /usr/tmp/CleSrv  			*
*Si (OUI & nbdata>0) sort temps restant & donnees serveur               *
*Si (OUI & nbdata<0  tue ressources serveur & lance sv     		*
*         SINON lance sv						*
************************************************************************/
#include "SV_include"

int main(int argc,char *argv[])
{
  long chaine[6]; /* temps fermeture et donnees serveur */
  FILE   *fp;
  int  nbdata,t_reste;
  
  
  if (argc < 2)
    {
      printf("\n!!!! Usage : SV  <+/-nombre de donnees>\n");
      exit(0);
   }
  nbdata=atoi(argv[1]);
  if (nbdata==0)
    {
      printf("\n!!!! nombre de donnees DIFFERENT de 0\n");
      exit(0);
    }
  sprintf(argv[1],"%d",abs(nbdata));
  if ((fp=fopen(CleServeur,"r")) != NULL) /* si le fichier existe */
    {
      fread(chaine,sizeof(long),6,fp);
      t_reste=chaine[0]-time(NULL);
      if (nbdata > 0)
	{
          printf("\n**************************************************\n");
          printf("!!!! Serveur en service ( pendant %d s) !!!\n",t_reste);
          printf("***********************i**************************\n");
          printf("Srv:ID messagerie publique Msqid = %ld\n", chaine[1]);
          printf("Srv:ID memoire Tampon      Tshmid= %ld \n", chaine[3]);
          printf("Srv:Pointeur Tampon        TPtr  = 0x%x\n", (unsigned) chaine[4]);
	  printf("Srv:Mutex   Semid = %ld \n", chaine[5]);
          exit(0);
	}
      else                /* Demande de tuer les ressources serveur */
	{
	  shmctl(chaine[2],IPC_RMID,0);   /*  segment memoire clients */
	  shmctl(chaine[3],IPC_RMID,0);   /*  segment memoire tampons */
	  msgctl(chaine[1],IPC_RMID,0);   /*  messagerie publique */
	  semctl(chaine[5],0, IPC_RMID,0);   /*  Mutex */
	  unlink(CleServeur);
	  unlink(CleClient);  /* delete fichiers */
	  printf("\n***************************************\n");
	  printf("!!! Ressources ancien serveur tuees !!!\n");
	  printf("***************************************\n");
	  sleep(1);
	}
    }
  
  /*  lance le serveur  */
  printf("\n******************************************\n");
  printf("!!! Lancement serveur (pendant %s s) !!!\n",argv[1]);
  printf("******************************************\n");
  if (execlp("./sv_zz","./sv_zz",argv[1],(char *)00) == -1)
    {
      printf("Erreur de execlp\n");
    }
  
  return 0;
}

