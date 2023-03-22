/* Fichier SV_connect.c  */


#include "SV_include"

void AlarmeTimeout(int s){};

void MsgTimout(int n)
{
signal(SIGALRM,AlarmeTimeout);
alarm(n);
}

/**********************************************
*	ENREGISTREMENT DES ARRIVEES ET DEPARTS
*
* ENTREE: msqid = identifiant de la messagerie
*	 : Cptr = pointeur sur la table des PID clients
*  SORTIE: ce processus doit etre detruit par le processus pere
*  RETOUR: -1 Si erreur sur msgrcv
*/

void AffichePIDSClients (int nb_cl, int *ptr)
{
	int j;

	printf("Number of clients = %d, max = %d\n", nb_cl, NCL_MAX);	
	printf("PID table: ");
	for (j = 0; j < NCL_MAX; j++)
		printf("%d  ", *(ptr+j));
	
	printf("\n");
}

void ReceptionClients(int msqid,int *Cptr)
{
dmsgbuf message;
int CPID;
int *ptr;
int nb_cl,i,k,erreur;

nb_cl=0; erreur=0;
*Cptr  = -1;
ptr = Cptr;
while(erreur>=0) {
   message.type = 0L;
   if ((erreur=msgrcv(msqid,&message,L_MSG,message.type,0)) <0)
      {
     if (errno==EINTR)
           erreur=0;
     else
           printf("Srv:Erreur %d RecepMessage:%s %ld\n",errno,message.txt,message.type);
     continue ;
       }
   printf("Srv:Reception MESSAGE:%s %ld\n",message.txt,message.type);
   switch (message.type)
         {
       case CONNECT  :
                 sscanf(message.txt,"%d",&CPID);

             if (nb_cl < NCL_MAX) {
                   message.type=CPID;
                   strcpy(message.txt,CleClient);
                   }
            else {
                  message.type = CPID;
                  strcpy(message.txt,""); /* message vide */
                  }
            if (msgsnd(msqid,&message,strlen(message.txt)+1,0) <0)
                  {
                  printf("Srv:erreur INFOC :%d",CPID);
              continue;
                  }
              message.type = ACK;
              MsgTimout(1);
           if (msgrcv(msqid,&message,L_MSG,message.type,0) <0)
                 {
                 printf("Srv:ACK non recu de %d \n",CPID);
              break;
                  }
           else  {
                 printf("Srv:Reception MESSAGE:%s %ld\n",message.txt,message.type);
                 sscanf(message.txt,"%d",&CPID);
                 ptr = Cptr ;
				for(k=0; k < NCL_MAX; k++)
				{
					if (*ptr <= 0) break;
					ptr ++;
				}

                *ptr = CPID;
                printf("Srv:%d connecte ACK :%s\n",CPID,message.txt);
                nb_cl++;
				
				AffichePIDSClients(nb_cl, Cptr);
                }
        break ;
      case  DECONNECT :
                sscanf(message.txt,"%d",&CPID);
                ptr = Cptr;
                i = 0;
             while ((*ptr != CPID) &&  (i < NCL_MAX))
				{
                        ptr ++;
						i ++;
				}
             if (i == NCL_MAX) {
                    printf("Srv:NB_clients : %d; DECONNECT erreur de pid :%d",nb_cl,CPID);
                continue;
                    }
            else {
                   *ptr = 0;
                    nb_cl-- ;
                    printf("Srv:NB_clients : %d; pid :%d deconnecte\n",nb_cl,CPID);

					AffichePIDSClients(nb_cl, Cptr);
				}
            break;
      default :
                printf("Srv : type inconnu %ld\n",message.type);
             break;
        }/* fin switch */
}/* fin while */
}/* fin procedure */
