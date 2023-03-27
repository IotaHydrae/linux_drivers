# Configure tools
MOD_DIR:=../..
SCRIPTS_DIR:=$(MOD_DIR)/scripts

# Tools
KLOGCAT:=$(SCRIPTS_DIR)/./klogcat.sh

# local kernel build dir
KERN_DIR:=/lib/modules/$(shell uname -r)/build

# users kernel dir
# KERN_DIR:=/home/user/linux

MODULE_NAME:=dht11

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

insmod: $(MODULE_NAME).ko
	sudo insmod $(MODULE_NAME).ko
	sh $(KLOGCAT)

rmmod: $(MODULE_NAME.ko)
	sudo rmmod $(MODULE_NAME)
	sh $(KLOGCAT)

log:
	sh $(KLOGCAT)

#tar:$(MODULE_NAME).ko

obj-m+=$(MODULE_NAME).o
