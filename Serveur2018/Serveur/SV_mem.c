/* Fichier SV_mem.c */

#include "SV_include"
#include "SV_mem.h"

/**********    ALLOCATION DE LA TABLE DES PID DES CLIENTS
*  ENTREE: Ptr = pointeur du pointeur du buffer client
*  SORTIE: le pointeur est renseigne
*  RETOUR: code erreur <0 ou identifcateur de la memoire
*/
int AllocMemoireClient(int **Ptr)
{
int *MemClientPtr;
int CLTshmid;
key_t key;

if (( key = ftok(CleServeur,'C')) < 0 )
   return CLEerr;
if ( (CLTshmid = shmget(key,NCL_MAX*sizeof(int),0644|IPC_CREAT)) <0)
   return SHMerr;
if ( (MemClientPtr = (int *)shmat(CLTshmid,NULL,0)) ==0 )
   return SHMerr;
*Ptr = MemClientPtr ;
return CLTshmid;
}

/*************   ALLOCATION D'UN TAMPON DE DONNEES
*  ENTREE: Ptr = pointeur du pointeur du tampon
*  SORTIE: le pointeur est renseigne
*  RETOUR: code erreur <0 ou identificateur de la memoire
*/
int AllocTampon(BUF **Ptr)
{
BUF *MemTamponPtr;
int CLTshmid;
key_t key;

if (( key = ftok(CleClient,'T')) < 0 )
   return CLEerr;
if ((CLTshmid = shmget(key,2*sizeof(BUF),0666|IPC_CREAT)) <0)
   return SHMerr;
if ((MemTamponPtr = (BUF *)shmat(CLTshmid,NULL,0)) == 0 )
   return SHMerr;
*Ptr = MemTamponPtr ;
return CLTshmid;
}

/*********************	DESALLOCATION DES TAMPONS
*  ENTREE: Tshmid,Cshmid = ID des memoires partagee
*  SORTIE: neant
*  RETOUR: code erreur <0 ou  0 ;
*/
int RelacheMemoires(int Tshmid, int Cshmid)
{
int ret = 0;

if (Tshmid)
   ret = shmctl(Tshmid,IPC_RMID,NULL);
if (Cshmid)
   ret += shmctl(Cshmid,IPC_RMID,NULL);
return ret;
}

