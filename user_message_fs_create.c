/**********************************************
* Init del modulo e montaggio del file system *
***********************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/version.h>
#include "user_messages_fs.h"

int single_mount; //variabile per evitare montaggi multipli
struct super_block *general_sb = NULL; //superblocco esposto a tutto il modulo

//parametri del modulo
unsigned long sys_call_table_address = 0x0;
module_param(sys_call_table_address, ulong, 0660);
int free_entries[15];
module_param_array(free_entries, int, NULL, 0660);

static struct super_operations umsg_fs_super_ops = {};
static struct dentry_operations umsg_fs_dentry_ops = {};

//esporre il superblocco alle system call
struct super_block *get_superblock(void){
    return general_sb;
}

//funzione di fill del superblocco
int umsg_fs_fill_super(struct super_block *sb, void *data, int silent){

    struct inode *root_inode;
    struct buffer_head *bh;
    struct umsg_fs_sb *sb_dev;
    struct timespec64 curr_time;
    uint64_t magic;
    uint64_t nblocks;
    struct umsg_fs_info *metadata; 
    struct umsg_fs_blockdata *blk_md;
    struct umsg_fs_block_info *prev;
    struct umsg_fs_block_info *tmp;
    int i;

    sb->s_magic = MAGIC;

    //ottieni device superblocco e controlla sia valido
    bh = sb_bread(sb, SB_BLOCK_NUMBER);
    if(bh == NULL){
        return -EIO;
    }
    sb_dev = (struct umsg_fs_sb *)bh->b_data;
    magic = sb_dev->magic;
    nblocks = sb_dev->nblocks;
    brelse(bh);

    if(magic != sb->s_magic){
        return -EBADF;
    }
    if(nblocks > NBLOCKS){
        return -EBADF;
    }

    //inizializzazione dei metadati in ram
    metadata = (struct umsg_fs_info *)kzalloc(nblocks * sizeof(struct umsg_fs_info), GFP_KERNEL);
    if(!metadata) return -ENOMEM;
    metadata->list_len = 0;
    INIT_LIST_HEAD_RCU(&(metadata->blk));
    metadata->nblocks = nblocks;
    metadata->max_timestamp = 0;
    mutex_init(&(metadata->write_mt));
    init_bitmask(metadata->mask, nblocks);
    sb->s_fs_info = (void *)metadata;


    sb->s_op = &umsg_fs_super_ops; //operazioni custom per superblock 

    //creazione root inode
    root_inode = iget_locked(sb, 0);
    if (root_inode == NULL){
        return -ENOMEM;
    }

    root_inode->i_ino = UMSG_FS_ROOT_INODE_NUM;
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5,12,0)
    inode_init_owner(&init_user_ns, root_inode, NULL, S_IFDIR); 
    #else
    inode_init_owner(root_inode, NULL, S_IFDIR);
    #endif
    root_inode->i_sb = sb;
    root_inode->i_op = &umsg_fs_inode_ops; //operazioni custom per inode
    root_inode->i_fop = &umsg_fs_dir_ops; //driver con ops del file

    root_inode->i_mode = S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP | S_IXOTH;

    ktime_get_real_ts64(&curr_time);
    root_inode->i_atime = curr_time;
    root_inode->i_mtime = curr_time;
    root_inode->i_ctime = curr_time;

    root_inode->i_private = NULL;

    sb->s_root = d_make_root(root_inode);
    if (sb->s_root==NULL)
        return -ENOMEM;
    sb->s_root->d_op = &umsg_fs_dentry_ops; //operazioni per la dentry

    unlock_new_inode(root_inode);

    //fill array di metadati
    for(i = 1; i < nblocks; i++){
        printk("Inizio la fill del blocco: %d\n", i); 
        bh = sb_bread(sb, i);
        if(!bh){
            return -EIO;
        }
        blk_md = (struct umsg_fs_blockdata *)(bh->b_data);
        if (blk_md->md.valid){
            printk("Il blocco è valido\n");
            metadata = (struct umsg_fs_info*)sb->s_fs_info;
            tmp = (struct umsg_fs_block_info *)kmalloc(sizeof(struct umsg_fs_block_info), GFP_KERNEL);
            if(!tmp) return -ENOMEM;
            tmp->data_lenght = blk_md->md.data_lenght;
            tmp->clock = blk_md->md.timestamp;
            tmp->id = blk_md->md.id;
            printk("Il blocco ha ordine: %lld\n", tmp->clock);
            
            if(!list_first_or_null_rcu(&(metadata->blk), struct umsg_fs_block_info, list)){
                list_add_tail_rcu(&(tmp->list), &(metadata->blk));
            }
            else{
                list_for_each_entry_rcu(prev, &(metadata->blk), list){
                    if(prev->clock > tmp->clock){
                        list_add_tail_rcu(&(tmp->list), &(prev->list));
                    }
                    else if(!list_next_or_null_rcu(&(metadata->blk), &(prev->list), struct umsg_fs_block_info, list)){
                        list_add_tail_rcu(&(tmp->list), &(metadata->blk));
                        break;
                    }
                    
                }
            }
            metadata->list_len++;
            if(tmp->clock > metadata->max_timestamp) metadata->max_timestamp = tmp->clock;
            set_id_bit(metadata->mask, tmp->id);
        }
        brelse(bh);
    }
    general_sb = sb;
    return 0;
}

//montaggio del filesystem
struct dentry *umsg_fs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data){
    
    struct dentry *ret;
    int ret1;
    printk("Entro in funzione mount\n");
   ret1 = __sync_bool_compare_and_swap(&single_mount, 0, 1 );
   if(!ret1) return ERR_PTR(-EBUSY);
    //montare block device generico e chiamare fill superblocco
    ret = mount_bdev(fs_type, flags, dev_name, data, umsg_fs_fill_super);

    if (unlikely(IS_ERR(ret))){
        printk("%s: error while mounting\n", MOD_NAME);
    }
    else {

        printk("%s mounted successfully\n", MOD_NAME);
    }
    return ret;
}

//smontaggio del filesystem
static void umsg_fs_kill_sb(struct super_block *sb){
    int i;
    struct umsg_fs_info *tmp = ((struct umsg_fs_info *)sb->s_fs_info);
    struct umsg_fs_block_info *cur = list_first_or_null_rcu(&(tmp->blk), struct umsg_fs_block_info, list);
    if(cur){
        struct umsg_fs_block_info *next = list_next_or_null_rcu(&(tmp->blk), &(cur->list), struct umsg_fs_block_info, list);
        if(next){
            for (i = 0; i < tmp->list_len; i++){
                kfree(cur);
                cur = next;
                next = list_next_or_null_rcu(&(tmp->blk), &(cur->list), struct umsg_fs_block_info, list);
                if(next == NULL) break;
            }
        }
        kfree(cur);
    }
    kfree(sb->s_fs_info);
    __sync_fetch_and_add(&single_mount, -1);
    kill_block_super(sb);
    printk("%s unmounted succesfully\n", MOD_NAME);
    return;
}

//struct per registare il filesystem
static struct file_system_type umsg_fs_type = {
    .owner = THIS_MODULE,
    .name = "user_message_fs",
    .mount = umsg_fs_mount, 
    .kill_sb = umsg_fs_kill_sb,
};

//init del modulo
static int umsg_fs_init(void){
    int ret;
    //registrare il file system
    ret = register_filesystem(&umsg_fs_type);
    if (ret == 0){
        printk("%s successfully registered\n", MOD_NAME);
        single_mount = 0;
        syscall_search(sys_call_table_address, free_entries);
    }
    else{
        printk("%s failed with error %d\n", MOD_NAME, ret);
    }

    return ret;
}

//exit del modulo
static void umsg_fs_exit(void){
    int ret;

    //deregistrare file system
    ret = unregister_filesystem(&umsg_fs_type);

    if (ret == 0){
        printk("%s successfully removed\n", MOD_NAME);
    }
    else{
        printk("%s can't be removed with error %d\n", MOD_NAME, ret);
    }
    return;
}

module_init(umsg_fs_init);
module_exit(umsg_fs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sara Da Canal");