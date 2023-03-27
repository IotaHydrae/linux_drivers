#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "dht11.h"

int main(int argc, char **argv)
{
	int fd;
	int ret;

	fd = open("/dev/dht11", O_NONBLOCK);
	if(fd<0){
		perror("open device failed!\n");
		return -1;
	}
	printf("fd: %d\n", fd);

	ret = ioctl(fd, DHT11_IOCREADBYTE);
	if(ret<0){
		perror("ioctl device failed!\n");
		
		return -1;
	}

	close(fd);
	return 0;
}
