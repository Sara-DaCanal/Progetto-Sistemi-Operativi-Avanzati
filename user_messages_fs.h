/*******************************************
* Header per il mio fantastico file system *
*******************************************/

#ifndef _UMESG_H
    #define _UMESG_H

    #include <linux/types.h>
    #include <linux/fs.h>
    #include "umsg.h"
    #define MOD_NAME "USER MESSAGE FS"
    #define SB_BLOCK_NUMBER 0
    #define INODE_BLOCK_NUMBER 1
    #define UMSG_FS_ROOT_INODE_NUM 10
   

    extern int single_mount;


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
        uint64_t nblocks;
        uint64_t mask[NBLOCKS/64 + 1];
        struct mutex write_mt;
        struct umsg_fs_block_info *blk;
    };


    extern const struct file_operations umsg_fs_ops;
    extern int syscall_search(unsigned long address, int free_entries[]);
    extern struct super_block *get_superblock(void);
    extern int put_data(struct super_block *sb, char * source, size_t size);
    extern int get_data(struct super_block *sb, int offset, char *destination, size_t size);
    extern int init_bitmask(uint64_t[], uint64_t);
    extern int set_id_bit(uint64_t[], uint64_t);
    extern int get_and_set(uint64_t[]);
#endif