#include <linux/module.h>

#include "user_messages_fs.h"

static int my_init(void){
    printk("Modulo inserito\n");
    printk("yeeeee\n");
    return 0;
}
static void my_exit(void){
    printk("Modulo rimosso\n");
    return;
}
module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sara Da Canal");