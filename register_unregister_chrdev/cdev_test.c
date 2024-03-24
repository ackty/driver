#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/fcntl.h>
int main(int argc, char **argv)
{
    int fd, cnt;
    char buf[256];
    printf("char device testing.\n");
    fd = open("/dev/my_char_dev", O_RDWR);           //打开字符设备
    if (fd == -1 )
    {
        printf("the char dev file cannot be opened.\n");
        return 1;
    }
    printf("input the data for kernel: ");
    scanf("%s", buf);                                //输入数据
    cnt = write(fd, buf, 256);                       //将输入数据写入设备
    if (cnt == 0)
        printf("Write Error! \n");
    cnt = read(fd, buf, 256);                        //从设备中读取数据
    if (cnt > 0)
        printf("read data from kernel is: %s\n", buf);
    else
        printf("read data error\n");
    close(fd);                                       //关闭设备
    printf("close the char dev file and test over\n");
    return 0;
}
