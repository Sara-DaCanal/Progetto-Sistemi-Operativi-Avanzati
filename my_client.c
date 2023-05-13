#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char *argv[]){
    int ret;
    int fd = open("./umsg_dir", O_RDWR);

    char buff[4096];
    ret = read(fd, buff, 10);
    ret = write(fd, "ciao", 5);
    printf("%d %d\n", ret, errno);
    printf("Questo è il mio buffer: %s\n", buff);
    close(fd);
    return 0;
}