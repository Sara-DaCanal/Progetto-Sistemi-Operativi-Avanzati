#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int main(int argc, char *argv[]){
    int ret;
    /*ret = syscall(134, "1234", strlen("1234"));
    printf("%d %d\n", errno, ret);
    char buff[100];
    ret = syscall(156, 5, buff, 100);
    printf("%d %d\n", errno, ret);
    printf("Questo è il mio buffer: %s\n", buff);
    ret = syscall(174, 4);
    printf("%d %d\n", errno, ret);

    ret = syscall(134, "12345", strlen("12345"));
    printf("%d %d\n", errno, ret);
    ret = syscall(156, 4, buff, 100);
    printf("%d %d\n", errno, ret);
    printf("Questo è il mio buffer: %s\n", buff);
    ret = syscall(174, 3);
    printf("%d %d\n", errno, ret);
    ret = syscall(156, 3, buff, 100);
    printf("%d %d\n", errno, ret);*/
    
   /* int fd = open("./umsg_dir/umsg_fs_file", O_RDONLY);

    
    char buff1[100];
    ret = read(fd, buff1, 10);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff1);*/

    printf("prima put\n");
    ret = syscall(134, "1234", strlen("1234"));
    printf("%d %d\n", errno, ret);
    printf("seconda put\n");
    ret = syscall(134, "Sono un blocco", strlen("Sono un blocco"));
    printf("%d %d\n", errno, ret);
    printf("terza put\n");
    ret = syscall(134, "Sono un blocco1234", strlen("Sono un blocco1234"));
    printf("%d %d\n", errno, ret);

    printf("Sono un invalid del blocco 3\n");
    ret = syscall(174, 2);
    printf("%d %d\n", errno, ret);

    printf("quarta put\n");
    ret = syscall(134, "ciao a tutti", strlen("ciao a tutti"));
    printf("%d %d\n", errno, ret);



   /* ret = read(fd, buff1, 100);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff1);

    ret = read(fd, buff1, 10);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff1);
    close(fd);*/
    
    return 0;
}