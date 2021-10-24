    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include<stdbool.h>
    #include<signal.h>
    #include<wait.h>
    #include<utmp.h>
    #include<sys/socket.h>
    bool Logger_Conf=0;
    int pid_client=0;
    #define FIFO_NAME "Client_FIFO"
    #define FIFO_NAME2 "Server_FIFO"
    #define Buffer_Size 500
    const char* Protocoale[10]=
    {  "login : ","Quit","get-logged-users","get-proc-info : ","logout"};
    void functieLogin(int fd,char* nume)
    {
        bool online;
        int pip[2];
        pipe(pip);
        int child;
        if((child=fork())>0)//parinte
        {
            close(pip[1]);//tatal doar citeste
            char string_valid[100];
            char string_aux[100];
            int aux=read(pip[0],string_aux,100);
            if(Logger_Conf==0)//Ca sa iau info din pipe daca e numele bun sau nu!si sa pun logger_conf 0/1
                {
                    read(pip[0],string_valid,100);
                    string_valid[1]='\0';
                    printf("asta am in string : %c\n",string_valid[0]);
                    Logger_Conf=string_valid[0]-'0';//aici mi-l face 1!!!
                    printf("asta am in string : %d\n",Logger_Conf);
                }
            string_aux[aux]='\0';
            wait(NULL);
            printf("Stringul auxiliar: %s\n",string_aux);
            write(fd,string_aux,strlen(string_aux));
            close(pip[0]);
        }
            else    if(child<0) perror("Nu merge forkul la login\n");
        else
        {   //sunt in Copil
            bool UtilValid=0;
            close(pip[0]);//copilul doar scrie
            FILE*fptr1;
            fptr1=fopen("Utilizatori.txt","r");
            char Nume_util[100];
            if(Logger_Conf==0)
            {//printf("%dinainte de while\n",Logger_Conf);
            while(!feof(fptr1))
            {
                fscanf(fptr1,"%s",Nume_util);//nu ia new lineul
                //printf("%s\n",Nume_util);
                if(strcmp(Nume_util,nume)==0)
                {
                    UtilValid=1;
                    break;
                }
            }
            if(UtilValid)
                {   
                    //Logger_Conf=true;
                   //printf("%d\n",Logger_Conf);
                    write(pip[1],"Utilizator valid! Bine ati venit!",strlen("Utilizator valid! Bine ati venit!"));
                    sleep(0.25);
                    write(pip[1],"1",1);
                }
            else
            {   
                write(pip[1],"Utilizator nevalid",strlen("Utilizator nevalid"));
                write(pip[1],"0",1);
            }
        }
            else
                write(pip[1],"Serverul este deja accesat de cineva sau utilizatorul este invalid!",strlen("Serverul este deja accesat de cineva sau utlizatorul este invalid!"));
            close(pip[1]);
            fclose(fptr1);
            exit(0);
        }
        
    }
    void functieLogged(int fd)
    {
        int sockp[2];
        socketpair(AF_UNIX,SOCK_STREAM,0,sockp);
        int child;
        if((child=fork())<0)
            perror("Eroare la fork()\n");
        else
            if(child>0)//parinte
                {
                   close(sockp[1]);
                   char Sir_Aux[Buffer_Size];
                   int aux=read(sockp[0],Sir_Aux,Buffer_Size);
                   wait(NULL);
                   Sir_Aux[aux]='\0';
                   //printf("Am primit %s din socket fiu\n",Sir_Aux);
                   write(fd,Sir_Aux,strlen(Sir_Aux));
                   close(sockp[0]);
                }else//copil
                {
                    if(Logger_Conf)
                    {
                    close(sockp[0]);
                    char Bloc[Buffer_Size]="";
                    char Sir_Aux[Buffer_Size]="";
                    struct utmp* Fol;
                    while((Fol=getutent())!=NULL)
    {
        sprintf(Sir_Aux,"\n Username:%s \n Hostname:%s \n Time entry was made %d sec %d msec",Fol->ut_user,Fol->ut_host,Fol->ut_tv.tv_sec,Fol->ut_tv.tv_usec);
        //printf("%s",Sir_Aux);
        strcat(Bloc,Sir_Aux);
    }
    Bloc[strlen(Bloc)]='\0';
    //printf("Asta e blocul meu:%s\n",Bloc);
    write(sockp[1],Bloc,strlen(Bloc));
}
                       
                    else
                    {   

                        write(sockp[1],"Nu sunteti logat",strlen("Nu sunteti logat"));
                        //write(sockp[1],"Nu sunteti log1t",strlen("Nu sunteti logat"));
                    }

            
                    close(sockp[1]);
                    exit(1);

                }
    }
    void functieLogout(int fd)
    {
        if(Logger_Conf==false)
                write(fd,"Nu sunteti logat pentru a va deloga!",strlen("Nu sunteti logat pentru a va deloga!"));
        else
        {
            Logger_Conf=false;
            write(fd,"V-ati delogat cu succes!",strlen("V-ati delogat cu succes!"));
        }

    }
    void functieQuit(int fd)
    {
            Logger_Conf=false;
            write(fd,"La revedere!",12);
    }
    void functieDefault(int fd)
    {
        write(fd,"Nu ati introdus o comanda valida",strlen("Nu ati introdus o comanda valida"));

    }
    void functieProc(int fd,const char*pid)
    {
        int pip[2];
        pipe(pip);
        int child;
        if((child=fork())>0)//parinte
        {
            close(pip[1]);//tatal doar citeste
            char string_aux[Buffer_Size];
            int aux=read(pip[0],string_aux,Buffer_Size);
            wait(NULL);
            string_aux[aux-1]='\0';
            printf("%s\n",string_aux);
            write(fd,string_aux,strlen(string_aux));
            close(pip[0]);
        }
            else
                if(child<0)
                    perror("Nu merge forkul la login\n");
        else
        {   
            close(pip[0]);//copilul doar scrie
            if(Logger_Conf)
            {
            //facem numele fisierului
            char Cale_Fisier[100]="/proc/";
            strcat(Cale_Fisier,pid);
            strcat(Cale_Fisier,"/status");
            //printf("%s\n",Cale_Fisier);
            //Avem calea catre fisier momentan
            int contor=1;
            char Parsare_Fisier[100];
            char Bloc[500]="\n";//sau Bloc[0]='\0';
            FILE*fptr1=fopen(Cale_Fisier,"r");
            while(contor<=18)
            {
                if(contor==1||contor==3||contor==7||contor==9||contor==18)
                {
                    fgets(Parsare_Fisier,100,fptr1);
                    //printf("%s",Parsare_Fisier);
                    strcat(Bloc,Parsare_Fisier);
                }
                else
                fgets(Parsare_Fisier,100,fptr1);
                ++contor;
            }
            //printf("%s",Bloc);
            write(pip[1],Bloc,strlen(Bloc));
            fclose(fptr1);
        }
            else
                write(pip[1],"Nu sunteti logat pentru a putea utliza aceasta comanda!",strlen("Nu sunteti logat pentru a putea utliza aceasta comanda!"));
            close(pip[1]);
            exit(0);
        }
    }
      
    int main()
    {
        char s[300];
        char v[300];
        int num, fd1,fd2;

        mknod(FIFO_NAME2,S_IFIFO | 0666, 0);
        mknod(FIFO_NAME, S_IFIFO | 0666, 0);

        printf("Nu exista cerere de deschidere...\n");
        fd1= open(FIFO_NAME, O_RDONLY);
        printf("S-a conectat un client:\n");
        do {if ((num = read(fd1, s, 300)) == -1) perror("Eroare la citirea din FIFO!");
            else { 
                s[num]='\0';  strcpy(v,s);    v[num]='\0';
        if(strncmp(s,"Client",6)==0)
            {   pid_client=atoi(s+6);
                Logger_Conf=false; 
            printf("Clientul are pidul %d\n",pid_client);//pentru anumite semnale..nu le-am main implementat dar cred ca va fii util candva
            }  
            else    if(strncmp(v,Protocoale[0],strlen(Protocoale[0]))==0)//pune Logger_Conf=0/1
                    {
                        //Logger_Conf=false;
                        //printf("Numele : \n");
                        fd2=open(FIFO_NAME2,O_WRONLY);
                        functieLogin(fd2,s+strlen(Protocoale[0]));
                        close(fd2);
                    }
            else    if(strncmp(v,Protocoale[1],strlen(Protocoale[1]))==0)
                        {
                        //printf("La revedere!\n");
                        fd2=open(FIFO_NAME2,O_WRONLY);
                        functieQuit(fd2);
                        close(fd2);
                        }
            else    if(strncmp(v,Protocoale[2],strlen(Protocoale[2]))==0)
                            {
                        //printf("Date necesare!\n");
                        fd2=open(FIFO_NAME2,O_WRONLY);
                        functieLogged(fd2);
                        close(fd2);
                            }
            else    if(strncmp(v,Protocoale[3],strlen(Protocoale[3]))==0)
                            {
                        //printf("Date din nou necesare!\n");
                        fd2=open(FIFO_NAME2,O_WRONLY);
                        functieProc(fd2,s+strlen(Protocoale[3]));
                        close(fd2);
                            }
            else    if(strncmp(Protocoale[4],v,strlen(Protocoale[4]))==0)
                            {
                        //printf("Logout cu succes!\n");
                        fd2=open(FIFO_NAME2,O_WRONLY);
                        functieLogout(fd2);
                        close(fd2);
                                    }
            else    if(s[0]=='\0');
                    else
                                        {
                                        //printf("%d\n",strlen(v));
                        //printf("Nu recunosc aceasta comanda\n");
                        fd2=open(FIFO_NAME2,O_WRONLY);
                        functieDefault(fd2);
                        close(fd2);
                            }
            }
        } while (1);
        close(fd1);
    }