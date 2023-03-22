/* Fichier SV_init.c */


#include "SV_include"
#include "SV_init.h"

/************* INTERRUPTION PERIODIQUE */
void Timeout(int signum)
{
signal(SIGALRM,Timeout);
alarm(1);
}

        /**************** 	GESTION DES FICHIERS DE CLE
        *
        *  SORTIE: les fichiers sont crees,
        *  CleServeur contient TempsFinal,Msqid,Cshmid,Tshmid & Tptr
        *  RETOUR: code erreur <0 ; 0 si pas d'erreur
        */
int GereFichierCle(long *pt_don)
{
FILE *fp;

if (pt_don != NULL)
   {
   if ((fp = fopen(CleServeur,"w")) == NULL)
      return CLEerr;
   if (*pt_don !=0 )
       fwrite(pt_don,sizeof(long),6,fp);
   fclose(fp);
   if ((fp = fopen(CleClient,"w")) == NULL)
      return CLEerr;
   fprintf(fp,"Serveur actif (Tampon Clients)\n");
   fclose(fp);

   return 0;
   }
else
  {
   unlink(CleServeur);	 /* tue le fichier CleSrv */
   unlink(CleClient);	 /* tue le fichier CleClt */
   }

return 0;
}
