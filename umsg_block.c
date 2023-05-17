#include <linux/buffer_head.h>
#include "user_messages_fs.h"

int put_data(struct super_block *sb, char * source, size_t size){
    struct buffer_head *bh;
    struct umsg_fs_info *metadata = sb->s_fs_info;
    struct umsg_fs_blockdata *my_data;
    struct umsg_fs_block_info *new_block;
    struct umsg_fs_block_info *last;
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
    my_data = (struct umsg_fs_blockdata *)bh->b_data;
    my_data->md.valid = 1;
    my_data->md.data_lenght = size;
    ret = copy_from_user(my_data->data, source, size);
    new_block = (struct umsg_fs_block_info *)kmalloc(sizeof(struct umsg_fs_block_info), GFP_KERNEL);
    if(!new_block) return -ENOMEM;
    new_block->valid = true;
    new_block->counter = 0;
    new_block->data_lenght = size;
    new_block->id = free_id;
    new_block->next = NULL;
    mutex_lock(&(metadata->write_mt));
    if(metadata->blk == NULL){
        new_block->clock = 1;
        __sync_bool_compare_and_swap(&(metadata->blk), NULL, new_block);
    }
    else{
        for(last = metadata->blk; last->next != NULL; last=last->next){
        }
        new_block->clock = last->clock + 1;
        __sync_bool_compare_and_swap(&(last->next), NULL, new_block);
    }
    mutex_unlock(&(metadata->write_mt));
    printk("%lld\n", (metadata->mask)[0]);
    printk("%lld\n", (metadata->mask)[0]);
    #ifdef SYNC
        sync_dirty_buffer(bh);
    #else
        mark_buffer_dirty(bh);
    #endif
    return free_id;
}

int get_data(struct super_block *sb, int offset, char *destination, size_t size){
    struct umsg_fs_info *md = (struct umsg_fs_info *)sb->s_fs_info;
    struct buffer_head *bh;
    struct umsg_fs_blockdata *my_data;
    size_t true_len;
    int ret;

    struct umsg_fs_block_info *cur;
    for(cur = md->blk; cur != NULL; cur = cur->next){
        __sync_fetch_and_add(&(cur->counter), 1);
        if(cur->id != offset){
            __sync_fetch_and_add(&(cur->counter), -1);
        }
        else{
            bh = sb_bread(sb, offset);
            if(!bh){
                __sync_fetch_and_add(&(cur->counter), -1);
                return -EIO;
            }
            my_data = (struct umsg_fs_blockdata *)bh->b_data;
            if(my_data->md.data_lenght < size) true_len = my_data->md.data_lenght;
            else true_len = size;
            ret = copy_to_user(destination, my_data->data, true_len);
            brelse(bh);
            __sync_fetch_and_add(&(cur->counter), -1);
            return true_len - ret;

        }
    }
    return -ENODATA;
}