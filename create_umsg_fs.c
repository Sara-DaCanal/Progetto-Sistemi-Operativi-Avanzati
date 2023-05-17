#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "umsg.h"

int main(int argc, char *argv[]){
    int fd;
    int nblocks;
    int i;
    struct umsg_fs_blockdata block_data;
    struct umsg_fs_sb sb;
    struct umsg_fs_inode inode;
    int ret;

    //check args
    if (argc != 3){
        printf("Usage: umsg_fs_create <device> nblocks\n");
        return -1;
    }

    //open device
    fd = open(argv[1], O_RDWR);
    if (fd == -1){
        printf("Errore nell'aprire il device\n");
        return -1;
    }
    nblocks = atoi(argv[2]); //numero di blocchi

    //set up superblock
    sb.magic = MAGIC;
    sb.nblocks = nblocks;

    ret = write(fd, (char *)&sb, sizeof(sb));

    if (ret !=UMSG_BLOCK_SIZE){
        printf("Couldn't write the superblock with error: %d. %d\n", errno, ret);
        close(fd);
        return -1;
    }

    printf("Superblock successfully written\n");

    inode.file_size = 100; //cambiare è una prova
    inode.inode_num = UMSG_FS_FILE_INODE_NUM;
    inode.nblocks = nblocks;

    ret = write(fd, (char *)&inode, sizeof(inode));
      if (ret !=UMSG_BLOCK_SIZE){
        printf("Couldn't write the inode block with error: %d. %d\n", errno, ret);
        close(fd);
        return -1;
    }    

    //write some data
    for (i = 2; i < nblocks/2; i++){
        block_data.md.valid = true;
        block_data.md.id=i;
        block_data.md.data_lenght = strlen("ciao");
        strcpy(block_data.data, "ciao");
        ret = write(fd, (char *)&block_data, sizeof(block_data));
        if (ret != UMSG_BLOCK_SIZE){
            printf("Couldn't write block %d with error: %d %d\n", i, errno, ret);
            close(fd);
            return -1;
        }
    }
        for (i; i < nblocks; i++){
        block_data.md.valid = false;
        block_data.md.id=i;
        block_data.md.data_lenght = 0;
        strcpy(block_data.data, "0");
        ret = write(fd, (char *)&block_data, sizeof(block_data));
        if (ret != UMSG_BLOCK_SIZE){
            printf("Couldn't write block %d with error: %d %d\n", i, errno, ret);
            close(fd);
            return -1;
        }
    }


    printf("device successfully formatted\n");

    close(fd);
    return 0;


}