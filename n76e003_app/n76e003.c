#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEV_NAME "/dev/n76e003"

// ioctl cmd
#define N76E003_IOC_MAGIC  'k'

#define N76E003_IOC_HEARTBEAT   _IO(N76E003_IOC_MAGIC, 1)
#define N76E003_IOC_SET_UPTIME  _IOW(N76E003_IOC_MAGIC, 2, int)

int main(int argc, char const *argv[])
{
	int ret, fd, num, arg;

	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("Open device fail!\n");
		return -1;
	}

    /*
	read(fd, &num, sizeof(int));
	printf("The num is: %d\n", num);

	printf("Please input a number written to testdriver: \n");
	scanf("%d", &num);

	write(fd, &num, sizeof(int));

	read(fd, &num, sizeof(int));
	printf("The num is: %d\n", num);
    */
    
    arg = 0xff;
    ret = ioctl(fd, N76E003_IOC_HEARTBEAT, &arg);
    if (ret < 0) {
        printf("ioctl: %d\n", ret);
    }
    
    sleep(10);
    
    arg = 0x01020304;
    ret = ioctl(fd, N76E003_IOC_SET_UPTIME, &arg);
    if (ret < 0) {
        printf("ioctl: %d\n", ret);
    }

	close(fd);
	return 0;
}