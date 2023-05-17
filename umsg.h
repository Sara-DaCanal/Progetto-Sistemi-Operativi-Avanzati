#ifndef _UMSG_H
    #define _UMSG_H


    #define UMSG_BLOCK_SIZE 4096
    #define UMSG_FS_FILE_INODE_NUM 1
    #define MAGIC 0x98765532

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
    struct __attribute__((packed)) umsg_fs_metadata{
        bool valid;
        uint64_t data_lenght;
        int id;
    };

    struct __attribute__((packed)) umsg_fs_blockdata{
        struct umsg_fs_metadata md;
        char data[4096 - sizeof(struct umsg_fs_metadata)];
    };
    
#endif