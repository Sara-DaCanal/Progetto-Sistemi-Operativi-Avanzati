/***********************
* Funzioni sui blocchi *
************************/
#include <linux/buffer_head.h>
#include "user_messages_fs.h"

//inserimento dati sul primo blocco libero
int internal_put_data(struct super_block *sb, char * source, size_t size){
    struct buffer_head *bh;
    struct buffer_head *bh1;
    struct umsg_fs_sb *sb_dev;
    struct umsg_fs_info *metadata = sb->s_fs_info;
    struct umsg_fs_blockdata *my_data;
    struct umsg_fs_block_info *new_block;
    int free_id;
    int ret;

    if(metadata->list_len == metadata->nblocks) return -ENOMEM;
    free_id = get_and_set(metadata->mask);
    printk("Il blocco da scrivere è %d\n", free_id);
    if(free_id == -1) return -ENOMEM;
           
    bh = sb_bread(sb, free_id);
    if(!bh){
        return -EIO;
    }
    bh1 = sb_bread(sb, SB_BLOCK_NUMBER);
    if(!bh){
        return -EIO;
    }
    sb_dev = (struct umsg_fs_sb *)bh1->b_data;
    set_id_bit(sb_dev->mask, free_id);
    my_data = (struct umsg_fs_blockdata *)bh->b_data;
    my_data->md.valid = 1;
    my_data->md.data_lenght = size;
   
    ret = copy_from_user(my_data->data, source, size);
    new_block = (struct umsg_fs_block_info *)kmalloc(sizeof(struct umsg_fs_block_info), GFP_KERNEL);
    if(!new_block) return -ENOMEM;
    new_block->id = free_id;
    mutex_lock(&(metadata->write_mt));
    metadata->max_timestamp++;
    sb_dev->order[free_id] = metadata->max_timestamp;
    new_block->clock = metadata->max_timestamp;
   
    list_add_tail_rcu(&(new_block->list), &(metadata->blk));
    metadata->list_len++;
    mutex_unlock(&(metadata->write_mt));
    mark_buffer_dirty(bh);
    mark_buffer_dirty(bh1);
    #ifdef SYNC
        sync_dirty_buffer(bh);
        sync_dirty_buffer(bh1);
    #endif
    return free_id;
}

//get data da un blocco
int internal_get_data(struct super_block *sb, int offset, char *destination, size_t size){
    struct umsg_fs_info *md = (struct umsg_fs_info *)sb->s_fs_info;
    struct buffer_head *bh;
    struct umsg_fs_blockdata *my_data;
    size_t true_len;
    int ret;
    struct umsg_fs_block_info *cur;
    rcu_read_lock();
    list_for_each_entry_rcu(cur, &(md->blk), list ){
        printk("%d\n", cur->id);
        if(cur->id == offset){
            
            bh = sb_bread(sb, offset);
            if(!bh){
                rcu_read_unlock();
                return -EIO;
            }
            my_data = (struct umsg_fs_blockdata *)bh->b_data;
            if(my_data->md.data_lenght < size) true_len = my_data->md.data_lenght;
            else true_len = size;
            ret = copy_to_user(destination, my_data->data, true_len);
            brelse(bh);
            rcu_read_unlock();
            return true_len - ret;

        }
    }
    rcu_read_unlock();
    return -ENODATA;
}

//invalidate di un blocco
int internal_invalidate(struct super_block *sb, int offset){
    struct umsg_fs_info *md = (struct umsg_fs_info *)sb->s_fs_info;
    struct umsg_fs_block_info *cur;
    struct umsg_fs_block_info *my_block;
    struct buffer_head *bh;
    struct buffer_head *bh1;
    struct umsg_fs_sb *sb_dev;
    rcu_read_lock();
    cur = list_first_or_null_rcu(&(md->blk), struct umsg_fs_block_info, list);
    if(!cur){
        rcu_read_unlock();
        return -ENODATA;
    }
    if(cur->id == offset){
        mutex_lock(&(md->write_mt));
        list_del_rcu(&(cur->list));
        mutex_unlock(&(md->write_mt));
    } 
    else{
        list_for_each_entry_rcu(cur, &(md->blk), list){
            my_block = list_next_or_null_rcu(&(md->blk), &(cur->list), struct umsg_fs_block_info, list);
            if(!my_block) break;
            if(my_block->id == offset){

                mutex_lock(&(md->write_mt));
                list_del_rcu(&(my_block->list));
                md->list_len--;
                mutex_unlock(&(md->write_mt));
            }
        }
    }
    rcu_read_unlock();
    bh = sb_bread(sb, offset);
    if(!bh) return -EIO;
    bh1 = sb_bread(sb, SB_BLOCK_NUMBER);
    if(!bh1) return -EIO;
    sb_dev = (struct umsg_fs_sb *)bh->b_data;
    reset_id_bit(sb_dev->mask, offset);
    ((struct umsg_fs_blockdata *)bh->b_data)->md.valid = false;
    mark_buffer_dirty(bh);
    mark_buffer_dirty(bh1);
    #ifdef SYNC
        sync_dirty_buffer(bh);
        sync_dirty_buffer(bh1);
    #endif
    synchronize_rcu();
    kfree(my_block);
    reset_id_bit(md->mask, offset);
    return 0;
}