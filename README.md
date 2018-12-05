# Linux ioctl 从应用到驱动
Linux ioctl 从应用程序到驱动例子

[示例代码](https://github.com/aystshen/linux_ioctl)

### 驱动
> 将示例代码中n76e003_driver复制到linux/drivers/misc目录下，修改相关Kconfig与Makefile文件  

1.头文件中定义ioctl命令
```
// ioctl cmd
#define N76E003_IOC_MAGIC  'k'

#define N76E003_IOC_HEARTBEAT   _IO(N76E003_IOC_MAGIC, 1)
#define N76E003_IOC_SET_UPTIME _IOW(N76E003_IOC_MAGIC, 2, int)

#define N76E003_IOC_MAXNR 2

struct n76e003_data {
    struct i2c_client *client;
    struct miscdevice n76e003_device;
};
```
2.实现ioctl函数
```
static int n76e003_dev_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	struct n76e003_data *n76e003 = container_of(filp->private_data,
							   struct n76e003_data,
							   n76e003_device);
	filp->private_data = n76e003;
	dev_info(&n76e003->client->dev,
		 "device node major=%d, minor=%d\n", imajor(inode), iminor(inode));

	return ret;
}


static long n76e003_dev_ioctl(struct file *pfile,
					 unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    int data = 0;
	struct n76e003_data *n76e003 = pfile->private_data;

    if (_IOC_TYPE(cmd) != N76E003_IOC_MAGIC) 
        return -EINVAL;
    if (_IOC_NR(cmd) > N76E003_IOC_MAXNR) 
        return -EINVAL;

    if (_IOC_DIR(cmd) & _IOC_READ)
        ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        ret = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (ret) 
        return -EFAULT;

    dev_info(&n76e003->client->dev,
                 "%s, (%x, %lx):\n", __func__, cmd,
                 arg);
    
	switch (cmd) {
    	case N76E003_IOC_HEARTBEAT:
            n76e003_heartbeat(n76e003->client);
    		break;
        
    	case N76E003_IOC_SET_UPTIME:
            if (copy_from_user(&data, (int *)arg, sizeof(int))) {
                dev_err(&n76e003->client->dev, 
                    "%s, copy from user failed\n", __func__);
                return -EFAULT;
            }
    		n76e003_set_uptime(n76e003->client, data);
    		break;
            
    	default:
    		return -EINVAL;
	}

	return 0;
}
```
3.定义file_operations
```
static const struct file_operations n76e003_dev_fops = {
	.owner = THIS_MODULE,
    .open = n76e003_dev_open,
	.unlocked_ioctl = n76e003_dev_ioctl
};
```
4.注册设备
```
static int n76e003_probe(struct i2c_client * client, const struct i2c_device_id * id)
{
    ...
    
    n76e003->n76e003_device.minor = MISC_DYNAMIC_MINOR;
	n76e003->n76e003_device.name = "n76e003";
	n76e003->n76e003_device.fops = &n76e003_dev_fops;

	ret = misc_register(&n76e003->n76e003_device);
	if (ret) {
		dev_err(&client->dev, "Failed misc_register\n");
		goto exit_misc_register;
	}

    ...
}
```

### 应用程序
> 如果是Android工程，将示例代码中n76e003_app复制到external目录下，并在Android根目录执行：make n76e003  

```
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
```

