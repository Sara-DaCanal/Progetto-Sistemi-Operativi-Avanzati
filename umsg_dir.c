#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "user_messages_fs.h"

//this iterate function just returns 3 entries: . and .. and then the name of the unique file of the file system
static int umsg_fs_iterate(struct file *file, struct dir_context* ctx) {

    //  printk("%s: we are inside readdir with ctx->pos set to %lld", MOD_NAME, ctx->pos);

        if(ctx->pos >= (2 + 1)) return 0;//we cannot return more than . and .. and the unique file entry

        if (ctx->pos == 0){
 //             printk("%s: we are inside readdir with ctx->pos set to %lld", MOD_NAME, ctx->pos);
                if(!dir_emit(ctx,".", 1, UMSG_FS_ROOT_INODE_NUM, DT_UNKNOWN)){
                        return 0;
                }
                else{
                        ctx->pos++;
                }

        }

        if (ctx->pos == 1){
  //            printk("%s: we are inside readdir with ctx->pos set to %lld", MOD_NAME, ctx->pos);
                //here the inode number does not care
                if(!dir_emit(ctx,"..", 2, 1, DT_UNKNOWN)){
                        return 0;
                }
                else{
                        ctx->pos++;
                }

        }
        if (ctx->pos == 2){
   //           printk("%s: we are inside readdir with ctx->pos set to %lld", MOD_NAME, ctx->pos);
                if(!dir_emit(ctx, FILE_NAME, strlen(FILE_NAME), UMSG_FS_FILE_INODE_NUM, DT_UNKNOWN)){
                        return 0;
                }
                else{
                        ctx->pos++;
                }

        }

        return 0;

}

//add the iterate function in the dir operations
const struct file_operations umsg_fs_dir_ops = {
    .owner = THIS_MODULE,
    .iterate = umsg_fs_iterate,
};
