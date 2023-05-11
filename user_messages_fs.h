/*******************************************
* Header per il mio fantastico file system *
*******************************************/

#ifndef _UMESG_H
    #define _UMESG_H

    #include <linux/types.h>
    #include <linux/fs.h>

    #define MOD_NAME "USER MESSAGE FS"
    #define MAGIC 0x98765532
    #define SB_BLOCK_NUMBER 0
    #define UMSG_FS_ROOT_INODE_NUM 1

    //definire campi necessari del superblocco
    struct umsg_fs_sb {
        uint64_t magic;
        uint64_t nblocks;
    };
#endif