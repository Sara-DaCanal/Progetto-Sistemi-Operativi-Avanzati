#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <sys/wait.h>
int main(int argc, char *argv[]){

    char *my_string;
    int ret;
    int pid, pid1, pid2;
    int fd;
    char buffer[100];
    printf("File per effettuare il test del modulo.\n");
    

    pid = fork();
    if(pid == 0){
        pid1 = fork();
        if(pid1 == 0){
            pid2 = fork();
            if(pid2 == 0){
                printf("Sono il terzo child e faro delle get\n");
                for(int i = 0; i < 10; i = i+2){
                    ret = syscall(156, i, buffer, 60);
                    if(ret == -1 && errno == ENODATA){
                        printf("Sono il terzo child e il blocco %d non è disponibile\n", i);
                    }
                    else if(ret == -1){
                        printf("Sono il terzo child è la get del blocco %d ha fallito con errore: %d\n", i, errno);
                        return -1;
                    }
                    else{
                        printf("Sono il terzo child e ho letto il blocco %d: \n%s\n", i, buffer);
                    }
                    sleep(3);
                }
                return 0;
            }
            printf("Sono il secondo child e farò delle invalidate\n");
            sleep(3);
            ret = syscall(174, 3);
            if(ret == 0){
                printf("Ho invalidato il blocco 3\n");
            }
            else if(errno == ENODATA){
                printf("Il blocco 2 non è disponibile\n");
            }
            sleep(4);
            ret = syscall(174, 4);
            if(ret == 0){
                printf("Ho invalidato il blocco 4\n");
            }
            else if(errno == ENODATA){
                printf("Il blocco 2 non è disponibile\n");
            }
            wait(NULL);
            return 0;
        }
        char r[300];
        printf("Sono il primo child e farò delle read\n");
        fd = open("umsg_dir/umsg_fs_file", O_RDONLY);
        printf("Sono il primo child e ho aperto il file\n");
        sleep(3);
        ret = read(fd, r, 50);
        printf("\nSono il primo child e ho letto %d caratteri:\n%s\n",ret, r);
        sleep(2);
        ret = read(fd, r, 100);
        printf("\nSono il primo child e ho letto %d caratteri:\n%s\n",ret, r);
        sleep(5);
        ret = read(fd, r, 300);
        printf("\nSono il primo child e ho letto:\n%s\n", r);
        close(fd);
        printf("Sono il primo child e ho chiuso il file\n");
        wait(NULL);
        return 0;
    }
    else{
        printf("Sono il parent e farò delle put\n");
        for(int i = 0; i < 11; i++){
            char s[100];

            strcpy(s, "Sono la put numero: ");
            sprintf(s+strlen("Sono la put numero: "), "%d", i);
            ret = syscall(134, s, strlen(s));
            if(ret == -1){
                if(errno == ENODATA){
                    printf("Sono il parent e la put %d ha fallito perché tutti i blocchi sono pieni\n", i);
                }else{
                printf("Sono il parent e la put %d ha fallito con errore %d\n", i, errno);
                return -1;
                }
            }
            printf("Sono il parent e ho effettuato una put sul blocco %d\n", ret);
            sleep(1);

        }
    }
    wait(NULL);

    return 0;
}