#include <linux/syscalls.h>
#include "user_messages_fs.h"
#include "./include/vtpmo.h"

//necessario avere il superblocco tra i parametri delle system call

__SYSCALL_DEFINEx(2, _put_data, char *, source, size_t, size){
    struct super_block *sb;
    if(!single_mount){
        printk("Single mount value %d\n", single_mount);
        return -ENODEV;
    }
    sb = get_superblock();
    return put_data(sb, source, size);
}
__SYSCALL_DEFINEx(3, _get_data, int, offset, char *, destination, size_t, size){
    struct super_block *sb;
    if(!single_mount){
        return -ENODEV;
    }
    sb = get_superblock();
    
    return get_data(sb, offset, destination, size);
}
__SYSCALL_DEFINEx(1, _invalidate_data, int, offset){
    struct super_block *sb;
    if(!single_mount){
        return -ENODEV;
    }
    sb = get_superblock();
    return invalidate(sb, offset);
}

long sys_put_data = (unsigned long) __x64_sys_put_data;
long sys_get_data = (unsigned long) __x64_sys_get_data;
long sys_invalidate_data = (unsigned long) __x64_sys_invalidate_data;

unsigned long cr0;

static inline void
write_cr0_forced(unsigned long val)
{
    unsigned long __force_order;

    /* __asm__ __volatile__( */
    asm volatile(
        "mov %0, %%cr0"
        : "+r"(val), "+m"(__force_order));
}

static inline void
protect_memory(void)
{
    write_cr0_forced(cr0);
}

static inline void
unprotect_memory(void)
{
    write_cr0_forced(cr0 & ~X86_CR0_WP);
}

int syscall_search(unsigned long address, int free_entries[]){
    	cr0 = read_cr0();
        unprotect_memory();
        ((unsigned long **)address)[free_entries[0]] = (unsigned long*)sys_put_data;
        ((unsigned long **)address)[free_entries[1]] = (unsigned long*)sys_get_data;
        ((unsigned long **)address)[free_entries[2]] = (unsigned long*)sys_invalidate_data;
        protect_memory();
        printk("Syscall installed\n");
    return 0;
}