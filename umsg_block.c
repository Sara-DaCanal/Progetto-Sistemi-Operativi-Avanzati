#include <linux/buffer_head.h>
#include "user_messages_fs.h"

int put_data(struct super_block *sb, char * source, size_t size){
    struct buffer_head *bh;
    struct umsg_fs_metadata *metadata = sb->s_fs_info;
    struct umsg_fs_blockdata *my_data;
    int i;
    int ret;
    for(i = 0; i < 8; i++){
        if(__sync_bool_compare_and_swap(&(metadata[i].valid), 0, 1)){
           
            printk("Trovato blocco %d libero\n", i+2);
            bh = sb_bread(sb, metadata[i].id);
            if(!bh){
                return -EIO;
            }
            printk("Blocco libero\n");
            my_data = (struct umsg_fs_blockdata *)bh->b_data;
            printk("b_data read\n");
            my_data->md.valid = 1;
            my_data->md.data_lenght = size;
            printk("strlen calcolata:%lld\n", my_data->md.data_lenght);
            ret = copy_from_user(my_data->data, source, size);
            printk("strcpy fatta %d\n", ret);

            mark_buffer_dirty(bh);

            break;
        }
    }
    return 0;
}