#include "CL_include"

/* drive.c */
#define BUF_SIZE 10000

int main()
{
    char buf[BUF_SIZE];

    printf("Je suis dans le DRIVE\n");
    strcat(buf, " ");

    while (1)
    {
        read(STDIN_FILENO, buf, BUF_SIZE);
        printf("\n\nMessage reçu: \n%s", buf);
    }

    return 0;
}
