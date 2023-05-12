#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char *argv[]){
    int ret;
    int fd = open("./umsg_dir", O_RDONLY);

    char buff[4096];
    ret = read(fd, buff, 4079);
    if (ret != 0){
        printf ("Errore: %d e fd:%d\n", errno, fd);
    }
    printf("Questo è il mio buffer: %s\n", buff);
    close(fd);
    return 0;
}