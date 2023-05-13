#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include "user_messages_fs.h"



ssize_t umsg_fs_read(struct file * filp, char __user * buf, size_t len, loff_t * off){
    struct buffer_head *bh = NULL;
    struct umsg_fs_blockdata *my_data;
    struct super_block *my_sb;
    struct umsg_fs_inode *my_inode;
    char *my_file;
    int copied;
    int ret;
    int i;
    size_t effective_len;

    bool new_stream = (bool)filp->private_data;
    if (new_stream)
        return 0;
    filp->private_data = (void *)true;

    //salvare il superblocco
    my_sb = filp->f_path.dentry->d_inode->i_sb;
    printk("Superblock successfully read\n");

    my_inode = filp->f_inode->i_private;
    
        printk("len: %ld\n", len);
    //controllare se l'offset è NULL
    if (off == NULL){
        if(len > my_inode->file_size)
            effective_len = my_inode->file_size;
        else
            effective_len = len;
         
    }else{  

    //controlli su len e offset
        if(*off >= my_inode->file_size)
            return 0;
        else if (*off +len > my_inode->file_size)
            len = my_inode->file_size - *off;
        printk("len: %ld e offset: %lld\n", len, *off);
    }

    //create file
    my_file = (char *)kmalloc(my_inode->file_size, GFP_KERNEL);
    if(!my_file){
        return -EIO;
    }
    printk("File successfully created\n");


    copied = 0; //lunghezza di byte copiati sul file
    for (i = 2; i < my_inode->nblocks; i++){
        bh = (struct buffer_head *)sb_bread(my_sb, i);
        printk("Ho letto il buffer per il nodo %d\n", i);
        if(!bh){
            return -EIO;
        }
        my_data = (struct umsg_fs_blockdata *)bh->b_data;
        if(my_data->valid){
            printk("%d\n", copied);
            strcpy(my_file+copied, "\n");
            copied++;
            strncpy(my_file+copied, my_data->data, my_data->data_lenght);
            printk("%s\n", my_file);
            copied += my_data->data_lenght;
        }
        brelse(bh);
        if (copied >= effective_len) break;
        
    }

    strcpy(my_file+copied, "\0");
    ret = copy_to_user(buf, my_file, copied);
    kfree(my_file);
    return len-ret;
}

int umsg_fs_open (struct inode *inode, struct file *filp){
    struct buffer_head *bh = NULL;
    bool new_stream = false;
    
    printk("File in apertura\n");

    //flag per lo stream 
    filp->private_data = (void *)new_stream; 
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