CONFIG_MODULE_SIG=n

obj-m += class_create_destroy.o

KDIR:=/lib/modules/$(shell uname -r)/build
PWD?=$(shell pwd)
 
all:
	make -C $(KDIR) M=$(PWD) modules 
	gcc cdev_test.c	-o cdev_test
driver:
	make -C $(KDIR) M=$(PWD) modules 
test:
	gcc cdev_test.c	-o cdev_test
 
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f cdev_test
