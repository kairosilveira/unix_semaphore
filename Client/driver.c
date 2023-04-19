#include "CL_include"

/* driver.c */
#define BUF_SIZE 10000

int main()
{
    char buf[BUF_SIZE];

    printf("\nDebut de reception des donnees\n");
    strcat(buf, " ");

    while (1)
    {
        read(STDIN_FILENO, buf, BUF_SIZE);
        printf("\n%s", buf);
    }

    return 0;
}
