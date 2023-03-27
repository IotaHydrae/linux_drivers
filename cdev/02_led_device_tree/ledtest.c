#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*Usage ./ledtest /dev/led on|off*/
int main(int argc, const char **argv)
{
	int fd;
	unsigned char val=1;

	fd = open(argv[1], O_RDWR);

	if(strcmp(argv[2], "on")==0)
		val = 1;
	else
		val = 0;
	
	write(fd, &val, 1);

	close(fd);
	return 0;
}
