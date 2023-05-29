/***************************
* Gestione maschere di bit *
****************************/
#include "user_messages_fs.h"

#define SINGLE_BIT_MASK 0x1
#define MASK_FULL 0xFFFFFFFFFFFFFFFF

uint64_t max;
int i;
static DEFINE_MUTEX(mt);

//set un bit a 1
int set_id_bit(uint64_t array[], uint64_t id){
    uint64_t array_i = id / 64;
    uint64_t bit_index = id % 64;
    if(array_i >= max) return -ENOBUFS; //da cambiare
    array[array_i] |= SINGLE_BIT_MASK << bit_index;
    return 0;
}

//set un bit a 0
int reset_id_bit(uint64_t array[], uint64_t id){
    uint64_t new_value;
    uint64_t array_i = id / 64;
    uint64_t bit_index = id % 64;
    if(array_i >= max) return -ENOBUFS;
    new_value = array[array_i] & ~(SINGLE_BIT_MASK << bit_index);
    __atomic_exchange_n(&(array[array_i]), new_value, __ATOMIC_SEQ_CST);
    return 0;
}

//init
int init_bitmask(uint64_t array[], uint64_t n){
    max = n/64 + 1;
    for(i = 0; i < max; i++){
        array[i] = 0;
    }
    set_id_bit(array, 0);
    return 0;
}

int check_bit(uint64_t array[], int i){
    int ret;
    uint64_t array_i = i / 64;
    uint64_t bit_index = i % 64;
    ret = array[array_i] & (SINGLE_BIT_MASK << bit_index);
    return ret; 
}

//trova il primo libero e set a 1
int get_and_set(uint64_t array[]){
    int ret;
    uint64_t pos, my_num;
    
    mutex_lock(&mt);
    for(i = 0; i < max; i++){
        my_num = array[i];
        if(my_num == 0){
            ret = set_id_bit(array, i*64);
            if(ret == -ENOBUFS){
                mutex_unlock(&mt);
                return -ENOBUFS;
            }
            mutex_unlock(&mt);
            return i*64;
        } else if( my_num == MASK_FULL){
            continue;
        } else {
            my_num = ~my_num;
            pos = 0;
            while(my_num > 0){
                if((my_num & 1) > 0){
                    ret = set_id_bit(array, i*64+pos);
                    if(ret == -ENOBUFS){
                        mutex_unlock(&mt);
                        return -ENOBUFS;
                    }
                    mutex_unlock(&mt);
                    return i*64+pos;
                }
                pos++;
                my_num = my_num >> 1;
            }  
        }
    }
    mutex_unlock(&mt);
    return -1;
}