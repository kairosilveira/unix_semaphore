/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
*	Module SV_def.h				*
*---------------------------------------------*
*	Definitions du serveur				*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define CleServeur "/tmp/CleSrv"
#define CleClient "/tmp/CleClt"

#define L_NOM_FICH  80
#define NVOIES   2	/* nombre de voies d'entree */
#define NCL_MAX  10	/* nombre maximum de clients */
#define BUF_SZ  3600	/* Taille des buffers d'entree */

/*---- Format des messages ------*/
#define L_MSG  200	/* longueur des messages */
#define CREATION  1      /* mode de gestion du fichier des cles */
#define CONNECT  1	/* demande de connexion */
#define DECONNECT  2	/* fin connexion */
#define INFOC  3        	/* information connexion */
#define ACK  4                  /* acquittement */
#define BUSY  5      	/* serveur occupe */

/*---- Codes d'erreur --------*/
#define NOERR_err       0
#define CLEerr          -100
#define SHMerr          -101
#define MSGerr          -102
#define SEMerr         -103

/*-- structure des messages --*/
typedef struct {
   long type;
   char txt[L_MSG];
}dmsgbuf;
 
/*-- structure des tampons --*/
typedef struct {
   int n;       /* indice tableau derniere donnee ecrite */
   int tampon[BUF_SZ];
}BUF;
