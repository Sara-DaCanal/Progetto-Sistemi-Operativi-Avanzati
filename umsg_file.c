#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include "user_messages_fs.h"



ssize_t umsg_fs_read(struct file * filp, char __user * buf, size_t len, loff_t * off){
    struct buffer_head *bh = NULL;
    struct umsg_fs_blockdata *my_data;
    struct umsg_fs_info *md;
    struct umsg_fs_block_info *blk;
    struct super_block *my_sb;
    struct umsg_fs_inode *my_inode;
    char *my_file;
    int copied;
    int ret;
    bool zero;
    size_t effective_len;
    uint64_t max_clock;

    uint64_t clock = *((uint64_t *)filp->private_data);
    zero = true;
    max_clock = 0;

    //salvare il superblocco
    my_sb = filp->f_path.dentry->d_inode->i_sb;
    printk("Superblock successfully read\n");

    md = (struct umsg_fs_info *)my_sb->s_fs_info;
    my_inode = filp->f_inode->i_private;

    if(len > my_inode->file_size)
         effective_len = my_inode->file_size;
    else
        effective_len = len;

    //create file
    printk("Taglia:%lld\n", my_inode->file_size);
    my_file = (char *)kmalloc(my_inode->file_size, GFP_KERNEL);
    if(!my_file){
        return -EIO;
    }
    printk("File successfully created\n");


    copied = 0; //lunghezza di byte copiati sul file
    printk("id del blocco %d\n", md->blk->id);
    for (blk = md->blk; blk != NULL; blk = blk->next){
        __sync_fetch_and_add(&(blk->counter), 1);
        if(blk->clock <= clock){
             __sync_fetch_and_add(&(blk->counter), -1);
        }
        else{
            if(blk->clock > max_clock) max_clock = blk->clock;
            zero = false;
            bh = (struct buffer_head *)sb_bread(my_sb, blk->id);
            printk("Ho letto il buffer per il nodo %d\n", blk->id);
            if(!bh){
                __sync_fetch_and_add(&(blk->counter), -1);
                return -EIO;
            }
            my_data = (struct umsg_fs_blockdata *)bh->b_data;
            
            
            strncpy(my_file+copied, my_data->data, my_data->md.data_lenght);
            copied += my_data->md.data_lenght;
            strcpy(my_file+copied, "\n");
            copied++;
            
            brelse(bh);
            __sync_fetch_and_add(&(blk->counter), -1);
            if (copied >= effective_len) break;
        }
        
    }
    if(zero){
        kfree(my_file);
        return 0;
    }
    *((uint64_t *)filp->private_data) = max_clock;
    printk("%lld\n", *((uint64_t *)filp->private_data));
    strcpy(my_file+copied, "\0");
    copied++;
    ret = copy_to_user(buf, my_file, copied);
    kfree(my_file);
    return len-ret;
}

int umsg_fs_open (struct inode *inode, struct file *filp){
    struct buffer_head *bh = NULL;
    uint64_t *clock;
    
    printk("File in apertura\n");

    //flag per lo stream 
    clock = (uint64_t *)kzalloc(sizeof(uint64_t), GFP_KERNEL);
    filp->private_data = (void *)clock; 
    filp->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE | FMODE_ATOMIC_POS | FMODE_CAN_WRITE);
    filp->f_mode |= FMODE_STREAM;
    
    //set inode
    bh = (struct buffer_head *)sb_bread(filp->f_path.dentry->d_inode->i_sb, INODE_BLOCK_NUMBER);
    if(!bh){
        return -EIO;
    }
    inode->i_private = (struct umsg_fs_inode *)bh->b_data;
    return 0;
}

int umsg_fs_release (struct inode *inode, struct file *filp){
    printk("File in fase di rilascio");
    kfree(filp->private_data);
    filp->private_data = NULL;
    inode->i_private = NULL;
    return 0;
}
const struct file_operations umsg_fs_ops = {
    .owner = THIS_MODULE,
    .read = umsg_fs_read,
    .open = umsg_fs_open,
    .release = umsg_fs_release
};