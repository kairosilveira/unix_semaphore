/* Fichier SV_sem.c */


#include "SV_include"

/****************	CREATION DU MUTEX
 *  ENTREE: Neant
 *  SORTIE: le Mutex est cree
 *  RETOUR: code erreur <0 ou identifcateur du Mutex
 */
int CreationMutex()
{
  key_t key;
  int  semid;
  if (( key = ftok(CleServeur,'S')) < 0 )
    return CLEerr;
  semid = semget(key, NVOIES, 0666|IPC_CREAT);
  return semid;
}


/*********************	DESTRUCTION DU MUTEX
 *  ENTREE: semid = identificateur de la famille de semaphores
 *  SORTIE: neant
 *  RETOUR: code erreur -1  ou 0
 */
int DestructionMutex(int semid)
{
  return (semctl(semid, 0, IPC_RMID, 0));
}

/*********************	Opération P
 *  ENTREE: semid = identificateur de la famille de semaphores
 *  SORTIE: neant
 *  RETOUR: 0 ou SEMerr
 */
int P(int semid, int voie)
{
  struct sembuf semoper;
  semoper.sem_num = voie;
  semoper.sem_op = -1;
  semoper.sem_flg = 0;
  
  if (semop(semid, &semoper, 1) < 0)
    {
      perror("Erreur P sur le Mutex");
      return SEMerr;
    }
  
  return 0;
}

/*********************	Opération V
 *  ENTREE: semid = identificateur de la famille de semaphores
 *  SORTIE: neant
 *  RETOUR: 0 ou SEMerr
 */
int V(int semid, int voie)
{
  struct sembuf semoper;
  semoper.sem_num = voie;
  semoper.sem_op = 1;
  semoper.sem_flg = 0;
  
  if (semop(semid, &semoper, 1) < 0)
    {
      perror("Erreur V sur le Mutex");
      return SEMerr;
    }
  
  return 0;
}


/*********************	Initialisation du MUTEX
 *  ENTREE: semid = identificateur de la famille de semaphores
 *  SORTIE: neant
 *  RETOUR: 0 ou SEMerr
 */
int Init_Mutex(int semid, int nb)
{
  struct sembuf semoper;
  int i;
  for(i = 0; i<NVOIES; i++)
    {
      semoper.sem_num = i;
      semoper.sem_op = nb;
      semoper.sem_flg = 0;
      
      if (semop(semid, &semoper, 1) < 0)
	{
	  printf("Erreur Initialisation du Mutex\n");
	  return SEMerr;
	}
    }
  return 0;
}

