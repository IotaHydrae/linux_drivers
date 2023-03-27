
# local kernel build dir
# KERN_DIR:=/lib/modules/$(shell uname -r)/build

# users kernel dir
#
DIR_PREFIX=/home/huazheng/hamsterbear/software/bsp
KERN_VER:=5.17
KERN_DIR:=$(DIR_PREFIX)/linux-$(KERN_VER)

MODULE_NAME:=ads1015

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

insmod: $(MODULE_NAME).ko
	sudo insmod $(MODULE_NAME).ko
	./klogcat.sh

rmmod: $(MODULE_NAME.ko)
	sudo rmmod $(MODULE_NAME)
	./klogcat.sh

obj-m+=$(MODULE_NAME).o
