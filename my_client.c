#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char *argv[]){
    int ret;
    int fd = open("./umsg_dir", O_RDONLY);

    char buff[100];
    char buff1[100];
    char buff2[100];
    ret = read(fd, buff, 10);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff);
    ret = read(fd, buff1, 40);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff1);
    ret = read(fd, buff2, 40);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff2);
    close(fd);
    return 0;
}