obj-m += user_message_fs.o
user_message_fs-objs += user_message_fs_create.o

all:
	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

insmod:
	insmod user_message_fs.ko

rmmod:
	rmmod user_message_fs