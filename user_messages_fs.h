/*******************************************
* Header per il mio fantastico file system *
*******************************************/

#ifndef _UMESG_H
    #define _UMESG_H

    #include <linux/types.h>
    #include <linux/fs.h>
    #define EXPORT_SYMTAB
    #define MOD_NAME "USER MESSAGE FS"
    #define MAGIC 0x98765532
    #define UMSG_BLOCK_SIZE 4096
    #define SB_BLOCK_NUMBER 0
    #define INODE_BLOCK_NUMBER 1
    #define UMSG_FS_ROOT_INODE_NUM 10
    #define UMSG_FS_FILE_INODE_NUM 1

    extern int single_mount;

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

    //struct del blocco in memoria
    struct umsg_fs_block_info{
        uint64_t counter;
        bool valid;
        uint64_t data_lenght;
        uint64_t clock;
        int id;
        struct umsg_fs_block_info *next;
    };

    //struct in memoria
    struct umsg_fs_info{
        uint64_t list_len;
        struct umsg_fs_block_info *blk;
    };


    extern const struct file_operations umsg_fs_ops;
    extern int syscall_search(unsigned long address, int free_entries[]);
    extern struct super_block *get_superblock(void);
    extern int put_data(struct super_block *sb, char * source, size_t size);
#endif