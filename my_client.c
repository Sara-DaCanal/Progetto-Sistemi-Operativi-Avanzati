#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int main(int argc, char *argv[]){
    int ret;
    ret = syscall(134, "1234", strlen("1234"));
    printf("%d %d\n", errno, ret);
    char buff[100];
    ret = syscall(156, 5, buff, 100);
    printf("%d %d\n", errno, ret);
    printf("Questo è il mio buffer: %s\n", buff);
    char buff1[2];
    ret = syscall(156, 2, buff1, 2);
    printf("%d %d\n", errno, ret);
    printf("Questo è il mio buffer: %s\n", buff1);
    char buff3[40];
    ret = syscall(156, 0, buff3, 40);
    printf("%d %d\n", errno, ret);
    printf("Questo è il mio buffer: %s\n", buff3);
    /*int fd = open("./umsg_dir", O_RDONLY);

    
    char buff1[100];
    char buff2[100];
    ret = read(fd, buff1, 100);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff1);
    close(fd);*/
    
    return 0;
}