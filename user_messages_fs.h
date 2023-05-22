/****************************
* Header per il file system *
*****************************/

#ifndef _UMESG_H
    #define _UMESG_H

    #include <linux/types.h>
    #include <linux/fs.h>
    #include <linux/rculist.h>
    #include "umsg.h"

    #define MOD_NAME "USER MESSAGE FS"
    #define FILE_NAME "umsg_fs_file"
    #define FILENAME_MAXLEN 255
    #define SB_BLOCK_NUMBER 0
    #define INODE_BLOCK_NUMBER 1
    #define UMSG_FS_ROOT_INODE_NUM 10
   
    extern int single_mount;

    //struct del blocco in memoria
    struct umsg_fs_block_info{
        uint64_t data_lenght;
        uint64_t clock;
        int id;
        struct list_head list;
    };

    //struct in memoria per il superblocco
    struct umsg_fs_info{
        uint64_t list_len;
        uint64_t nblocks;
        uint64_t max_timestamp;
        uint64_t mask[NBLOCKS/64 + 1];
        struct mutex write_mt;
        struct list_head blk;
    };

    extern const struct file_operations umsg_fs_ops;
    extern const struct file_operations umsg_fs_dir_ops;
    extern const struct inode_operations umsg_fs_inode_ops;
    extern int syscall_search(unsigned long address, int free_entries[]);
    extern struct super_block *get_superblock(void);
    extern int internal_put_data(struct super_block *sb, char * source, size_t size);
    extern int internal_get_data(struct super_block *sb, int offset, char *destination, size_t size);
    extern int internal_invalidate(struct super_block *sb, int offset);
    extern int init_bitmask(uint64_t[], uint64_t);
    extern int set_id_bit(uint64_t[], uint64_t);
    extern int reset_id_bit(uint64_t[], uint64_t);
    extern int get_and_set(uint64_t[]);
#endif