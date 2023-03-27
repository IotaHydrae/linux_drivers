
# local kernel build dir
#KERN_DIR:=/lib/modules/$(shell uname -r)/build

# users kernel dir
KERN_DIR:=/home/developer/sources/T113-PRO-Longan-SDK/T113-Pro_Longan-SDK/out/kernel/build
#ERN_DIR:=/home/developer/sources/T113-PRO-Longan-SDK/T113-Pro_Longan-SDK/kernel/linux-5.4

#MODULE_NAME:=virt-tty
MODULE_NAME:=ssd1681

all:
	make -C $(KERN_DIR) ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-  M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

insmod: $(MODULE_NAME).ko
	sudo insmod $(MODULE_NAME).ko
	./klogcat.sh

rmmod: $(MODULE_NAME.ko)
	sudo rmmod $(MODULE_NAME)
	./klogcat.sh

CFLAGS_$(MODULE_NAME).o := -DDEBUG
obj-m+=$(MODULE_NAME).o
