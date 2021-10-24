    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include<stdbool.h>
    #define FIFO_NAME "Client_FIFO"
    #define FIFO_NAME2 "Server_FIFO"
    #define Buffer_Size 500
    int main()
    {
        char s[Buffer_Size];
        int num, fd1,fd2;
        pid_t pid=getpid();
      

        //printf("Serverul este inactiv momentan..\n");
        fd1=open(FIFO_NAME,O_WRONLY);
        printf("Serverul este activ acum!\n");
        bool IntrareNouaInSistem=1;
        if(IntrareNouaInSistem)
            {
                char pid_char[16];
                sprintf(pid_char,"Client%d",pid);
                write(fd1,pid_char,strlen(pid_char));
                IntrareNouaInSistem=0;
            }
        while (fgets(s,100,stdin)) {
                s[strlen(s)-1]='\0';
            if ((num = write(fd1, s, strlen(s))) == -1)
                perror("Problema la scriere in FIFO!");
            else
                {
                fd2=open(FIFO_NAME2,O_RDONLY);
                char info_aux[Buffer_Size];
                int aux=read(fd2,info_aux,Buffer_Size);
                info_aux[aux]='\0';
                printf("%d octeti: %s\n",aux,info_aux);
                if(strcmp("La revedere!",info_aux)==0)
                    {
                        close(fd2);
                        close(fd1);
                        exit(0);
                    }
                close(fd2);
                 }
        }
        close(fd1);
    }