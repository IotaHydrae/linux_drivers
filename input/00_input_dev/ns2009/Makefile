KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88
KERN_DIR = /home/yuanxin/source/linux-4.14.240

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

# obj-m	+= bonbon_oled.o
obj-m += ns2009.o
