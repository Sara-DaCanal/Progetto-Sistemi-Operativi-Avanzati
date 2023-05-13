/*******************************************
* Header per il mio fantastico file system *
*******************************************/

#ifndef _UMESG_H
    #define _UMESG_H

    #include <linux/types.h>
    #include <linux/fs.h>

    #define MOD_NAME "USER MESSAGE FS"
    #define MAGIC 0x98765532
    #define UMSG_BLOCK_SIZE 4096
    #define SB_BLOCK_NUMBER 0
    #define INODE_BLOCK_NUMBER 1
    #define UMSG_FS_ROOT_INODE_NUM 10
    #define UMSG_FS_FILE_INODE_NUM 1


    //definire campi necessari del superblocco
    struct __attribute__((packed)) umsg_fs_sb {
        uint64_t magic;
        uint64_t nblocks;

        char padding[4096-(2*sizeof(uint64_t))];
    };

    struct __attribute__((packed)) umsg_fs_inode{
        uint64_t inode_num;
        uint64_t file_size;
        uint64_t nblocks;
        char padding[4096-(3*sizeof(uint64_t))];
    };

    //struct per ogni blocco
    struct __attribute__((packed)) umsg_fs_blockdata{
        bool valid;
        uint64_t data_lenght;
        int id;
        char data[4096-(sizeof(bool)+sizeof(uint64_t)+sizeof(int))];
    };

    extern const struct file_operations umsg_fs_ops;
#endif