#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char *argv[]){
    int ret;
    //ret = syscall(134, "1234", 10);
   // printf("%d %d\n", errno, ret);
    int fd = open("./umsg_dir", O_RDONLY);

    char buff[100];
    char buff1[100];
    char buff2[100];
    ret = read(fd, buff1, 100);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff1);
    close(fd);
    
    return 0;
}