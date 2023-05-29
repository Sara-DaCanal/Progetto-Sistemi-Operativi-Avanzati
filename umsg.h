/****************
* Libreria user *
*****************/
#ifndef _UMSG_H
    #define _UMSG_H


    #define UMSG_BLOCK_SIZE 4096
    #define UMSG_FS_FILE_INODE_NUM 1
    #define MAGIC 0x98765532

    #ifndef NBLOCKS 
        #define NBLOCKS 10
    #endif
    #if NBLOCKS > 500
        NBLOCKS = 500
    #endif
    

    //definire campi necessari del superblocco
    struct __attribute__((packed)) umsg_fs_sb {
        uint64_t magic;
        uint64_t nblocks;
        uint64_t mask[NBLOCKS/64 + 1];
        uint64_t order[NBLOCKS];
        char padding[4096-((NBLOCKS/64 + 3 + NBLOCKS)*sizeof(uint64_t))];
    };


    //struct metadati per ogni blocco
    struct __attribute__((packed)) umsg_fs_metadata{
        bool valid;
        uint64_t data_lenght;
        int id;
    };

    //struct per ogni blocco
    struct __attribute__((packed)) umsg_fs_blockdata{
        struct umsg_fs_metadata md;
        char data[4096 - sizeof(struct umsg_fs_metadata)];
    };
    
#endif