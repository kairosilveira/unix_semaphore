#include "CL_include"

/* drive.c */
#define BUF_SIZE 10000

int main() {
    char buf[BUF_SIZE];
    int nread;

    printf("Estou no DRIVE\n");
    strcat(buf," ");

    while (1) {
        nread = read(STDIN_FILENO, buf, BUF_SIZE);
        printf("\n\nMensagem recebida: \n%s", buf);
        //for (int i =0; i<5; i++){
        //    scanf("%s",buf);
        //}
		
		//printf("DRIVER.exe: \n %s \n",buf);
        //strcat(buf," ");
    }

    return 0;
}




