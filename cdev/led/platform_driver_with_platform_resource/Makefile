
# local kernel build dir
KERN_DIR:=/lib/modules/$(shell uname -r)/build

# users kernel dir
# KERN_DIR:=/home/user/linux

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

# obj-m+=eeprom.o
# obj-m+=test_eep.o

obj-m+=led.o
obj-m+=led_resource.o
