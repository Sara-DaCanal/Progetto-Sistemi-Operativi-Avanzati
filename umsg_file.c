/********************
* Funzioni sul file *
*********************/
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include "user_messages_fs.h"


//funzione read
ssize_t umsg_fs_read(struct file * filp, char __user * buf, size_t len, loff_t * off){
    struct buffer_head *bh = NULL;
    struct umsg_fs_blockdata *my_data;
    struct umsg_fs_info *md;
    struct umsg_fs_block_info *blk;
    struct super_block *my_sb;
    char *my_file;
    int copied;
    int ret;
    bool zero;
    int block_to_read;
    uint64_t max_clock;

    uint64_t clock = *((uint64_t *)filp->private_data);
    zero = true;
    max_clock = 0;

    //salvare il superblocco
    my_sb = filp->f_path.dentry->d_inode->i_sb;
    printk("Superblock successfully read\n");

    md = (struct umsg_fs_info *)my_sb->s_fs_info;

    block_to_read = len / UMSG_BLOCK_SIZE + 1;
    if(block_to_read > md->list_len)
        block_to_read = md->list_len;

    //create file
    my_file = (char *)kmalloc(block_to_read*UMSG_BLOCK_SIZE, GFP_KERNEL);
    if(!my_file){
        return -EIO;
    }
    printk("File successfully created\n");


    copied = 0; //lunghezza di byte copiati sul file
    rcu_read_lock();
    list_for_each_entry_rcu(blk, &(md->blk), list){
        if(copied + blk->data_lenght > len) break;
        if(blk->clock > clock){
            if(blk->clock > max_clock) max_clock = blk->clock;
            zero = false;
            bh = (struct buffer_head *)sb_bread(my_sb, blk->id);
            printk("Ho letto il buffer per il nodo %d\n", blk->id);
            if(!bh){
                rcu_read_unlock();
                return -EIO;
            }
            my_data = (struct umsg_fs_blockdata *)bh->b_data;
            strncpy(my_file+copied, my_data->data, my_data->md.data_lenght);
            copied += my_data->md.data_lenght;
            strcpy(my_file+copied, "\n");
            copied++;
            
            brelse(bh);
        }
        
    }
    rcu_read_unlock();
    if(zero){
        kfree(my_file);
        return 0;
    }
    *((uint64_t *)filp->private_data) = max_clock;
    strcpy(my_file+copied, "\0");
    copied++;
    ret = copy_to_user(buf, my_file, copied);
    kfree(my_file);
    return len-ret;
}

//funzione open
int umsg_fs_open (struct inode *inode, struct file *filp){
    uint64_t *clock;
    
    printk("File in apertura\n");

    //flag per lo stream 
    clock = (uint64_t *)kzalloc(sizeof(uint64_t), GFP_KERNEL);
    filp->private_data = (void *)clock; 
    filp->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE | FMODE_ATOMIC_POS | FMODE_CAN_WRITE);
    filp->f_mode |= FMODE_STREAM;
    
    return 0;
}

//funzione release
int umsg_fs_release (struct inode *inode, struct file *filp){
    printk("File in fase di rilascio");
    kfree(filp->private_data);
    filp->private_data = NULL;
    return 0;
}

struct dentry *umsg_fs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags){
    struct super_block *sb = parent_inode->i_sb;
    struct inode *my_inode = NULL;

    if(!strcmp(child_dentry->d_name.name, FILE_NAME)){
        my_inode = iget_locked(sb, 1);
        if(!my_inode) return ERR_PTR(-ENOMEM);

        if(!(my_inode->i_state & I_NEW)){
            return child_dentry;
        }

        inode_init_owner(&init_user_ns, my_inode, NULL, S_IFREG);
        my_inode->i_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP | S_IXOTH;
        my_inode->i_fop = &umsg_fs_ops;

        set_nlink(my_inode, 1);
        d_add(child_dentry, my_inode);
        dget(child_dentry);
        unlock_new_inode(my_inode);
        return child_dentry;
    }
    return NULL;
}
const struct file_operations umsg_fs_ops = {
    .owner = THIS_MODULE,
    .read = umsg_fs_read,
    .open = umsg_fs_open,
    .release = umsg_fs_release
};

const struct inode_operations umsg_fs_inode_ops = {
    .lookup = umsg_fs_lookup
};