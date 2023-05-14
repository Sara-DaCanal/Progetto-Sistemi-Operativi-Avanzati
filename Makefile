obj-m += user_message_fs.o
user_message_fs-objs += user_message_fs_create.o umsg_file.o
EXTRA_CFLAGS:= -D NBLOCKS=10

all:
	gcc create_umsg_fs.c -o create_umsg_fs
	gcc my_client.c -o my_client
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm create_umsg_fs
	rm my_client
	rm -r umsg_dir

insmod:
	insmod user_message_fs.ko

rmmod:
	rmmod user_message_fs

format:
	dd bs=4096 count=10 if=/dev/zero of=image1
	./create_umsg_fs image1 10
	mkdir umsg_dir

mount-fs:
	mount -o loop -t user_message_fs image ./umsg_dir/

umount-fs:
	umount umsg_dir