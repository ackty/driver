/*头文件引用*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

/*宏定义及全局变量定义*/
#define MEM_MALLOC_SIZE    4096                       //缓冲区大小
#define MEM_MAJOR       245                           //主设备号
#define MEM_MINOR          0                          //次设备号
char *mem_spvm;                                      //缓冲区指针，指向内存区
struct cdev *mem_cdev;                               //设备对象指针
struct class *mem_class;                             //设备类指针

static int  __init  cdev_add_del_init(void);          //模块加载函数声明
static void  __exit  cdev_add_del_exit(void);         //模块卸载函数声明
static int mem_open(struct inode *ind, struct file *filp);     //设备打开函数声明
static int mem_release(struct inode *ind, struct file *filp); //设备关闭函数声明

/*设备读函数声明*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *fpos);

/*设备写函数声明*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size,loff_t *fpos);

/*定义设备驱动文件结构体*/
struct file_operations mem_fops =
{
    .open = mem_open,                               //打开设备函数
    .release = mem_release,                         //关闭设备函数
    .read = mem_read,                               //读设备数据函数
    .write = mem_write,                             //向设备写数据函数
};

int __init cdev_add_del_init (void)
{
    int res;
    printk("into the cdev_add_del_init\n");
    int devno = MKDEV(MEM_MAJOR, MEM_MINOR);        //创建设备号，主设备号与次设备号
    mem_spvm = (char *)vmalloc(MEM_MALLOC_SIZE); //开辟内存缓冲区
    if (mem_spvm == NULL)
        printk("vmalloc failed! \n");
    else
        printk("vmalloc successfully! addr=0x%x\n", (unsigned int)mem_spvm);
    mem_cdev = cdev_alloc();                       //动态分配一个新的字符设备对象
    if (mem_cdev == NULL)
    {
        printk("cdev_alloc failed! \n");
        return 0;
    }
    cdev_init(mem_cdev, &mem_fops);                 //初始化字符设备对象
    mem_cdev->owner = THIS_MODULE;                  //初始化设备持有者
    res = cdev_add(mem_cdev, devno, 1);             //将字符设备加入内核系统
    if (res)
    {
        cdev_del(mem_cdev);                         //添加失败，删除设备驱动
        mem_cdev = NULL;                            //添加失败，释放缓冲区
        printk("cdev add Error\n");
    }
    else
    {
        printk("cdev add OK\n");
    }
    printk("out the cdev_add_del_init\n");
    return 0;
}

void __exit cdev_add_del_exit (void)
{
    printk("into cdev_add_del_exit\n");
    if (mem_cdev != NULL)
        cdev_del(mem_cdev);                         //从内核中将设备删除
    printk("cdev del OK\n");
    if (mem_spvm != NULL)
        vfree(mem_spvm);                            //释放缓冲区空间
    printk("vfree ok! \n");
    printk("out cdev_add_del_exit\n");
}

/*设备打开函数定义*/
int mem_open(struct inode *ind, struct file *filp)
{
    printk("open vmalloc space\n");
    try_module_get(THIS_MODULE);                  //模块引用计数器自加
    printk("open vmalloc space success\n");
    return 0;
}

/*设备读函数定义*/
ssize_t mem_read(struct file *filp, char *buf, size_t size, loff_t *lofp)
{
    int res = -1;
    char *tmp;
    printk("copy data to the user space\n");
    tmp = mem_spvm;
    if (size > MEM_MALLOC_SIZE)                   //判断读取数据的大小
        size = MEM_MALLOC_SIZE;
    if (tmp != NULL)
        res = copy_to_user(buf, tmp, size);       //将内核输入写入用户空间
    if (res == 0)
    {
        printk("copy data success and the data is:%s\n", tmp);    //显示读取的数据
        return size;
    }
    else
    {
        printk("copy data fail to the user space\n");
        return 0;
    }
}

/*设备写函数定义*/
ssize_t mem_write(struct file *filp, const char *buf, size_t size, loff_t *lofp)
{
    int res = -1;
    char *tmp;
    printk("read data from the user space\n");
    tmp = mem_spvm;
    if (size > MEM_MALLOC_SIZE)                    //判断输入数据的大小
        size = MEM_MALLOC_SIZE;
    if (tmp != NULL)
        res = copy_from_user(tmp, buf, size);      //将用户输入数据写入内核空间
    if (res == 0)
    {
        printk("read data success and the data is:%s\n", tmp);    //显示写入的数据
        return size;
    }
    else
    {
        printk("read data from user space fail\n");
        return 0;
    }
}

/*设备关闭函数定义*/
int mem_release(struct inode *ind, struct file *filp)
{
    printk("close vmalloc space\n");
    module_put(THIS_MODULE);                       //模块引用计数器自减
    printk("close vmalloc space success\n");
    return 0;
}

module_init(cdev_add_del_init);                   //模块加载
module_exit(cdev_add_del_exit);                   //模块卸载

