#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "user_messages_fs.h"



ssize_t umsg_fs_read(struct file * filp, char __user * buf, size_t len, loff_t * off){
    struct buffer_head *bh = NULL;
    struct umsg_fs_blockdata *my_data;
   // char my_file[];
    int ret;
    int i;
    for (i = 2; i < 3; i++){
        bh = (struct buffer_head *)sb_bread(filp->f_path.dentry->d_inode->i_sb, i);
        if(!bh){
            printk("ERRORE\n");
            return -EIO;
        }
        my_data = (struct umsg_fs_blockdata *)bh->b_data;
        if(my_data->valid){
            printk("trovati dati validi");
            ret = copy_to_user(buf, my_data->data, sizeof(my_data->data));
        }
        brelse(bh);
    }
    printk("File size is: %s\n", (char *)(filp->private_data));
    return ret;
}

int umsg_fs_open (struct inode *inode, struct file *filp){
    filp->private_data = (void *)"ciao"; //che intende ad aprirlo come stream?
    return stream_open(inode, filp);
}

/*int umsg_fs_release (struct inode *inode, struct file *filp){
    filp->private_data = NULL;
    printk("File in fase di rilascio");
    return generic_file_release(inode, filp);
}*/
const struct file_operations umsg_fs_ops = {
    .owner = THIS_MODULE,
    .read = umsg_fs_read,
    .open = umsg_fs_open
};