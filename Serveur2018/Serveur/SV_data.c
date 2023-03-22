/* fichier SV_data.c */


#include "SV_include"

/****************************    SIGNALISATION DONNEE NOUVELLE
* ENTREE: Voie : numero de la voie renseignee
*  SORTIE: tous les clients ont recu un signal SIGUSR1 ou SIGUSR2
*  RETOUR: neant
*/
void SignaleDonnee(int Voie, int *Cptr)
{
int SIG,h,*ptr;

ptr = Cptr ;
SIG = ( Voie <2 ? SIGUSR1 : SIGUSR2 );

h = 0;
while (h < NCL_MAX )
   {
   if ( *ptr >0 )
        kill(*ptr,SIG) ;

   ptr++;
	h ++;
   }
}
        

/************************* GENERATEUR DE DONNEES
* ENTREE: Tptr = pointeur du tampon des donnees
*  SORTIE: une donnee est cree sur la voie 1 ou sur la voie 2
*  RETOUR: numero de la voie renseignee
*/
int GenereData(BUF *Tptr, int va)
{
  int n;
  n = (Tptr + va)->n;
  n = (n+1)%BUF_SZ;
  (Tptr+va)->tampon[n] = va==0 ? BUF_SZ - n  : n ;
  (Tptr+va)->n = n;
  return va+1;
}

