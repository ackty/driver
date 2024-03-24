/*头文件引用*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <asm/unistd.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");

/*宏定义及全局变量定义*/
#define MEM_MALLOC_SIZE 4096       //缓冲区大小
#define MEM_MAJOR        245         //主设备号
#define MEM_MINOR        0           //次设备号
char *mem_spvm;                     //缓冲区指针，指向内存区
struct class *mem_class;            //设备类指针
struct device *dev;                 //逻辑设备类指针

static int  __init  device_initialize_add_del_init(void);       //模块加载函数声明
static void  __exit device_initialize_add_del_exit(void);       //模块卸载函数声明
static int mem_open(struct inode *ind, struct file *filp);    //设备打开函数声明
static int mem_release(struct inode *ind, struct file *filp); //设备关闭函数声明

/*设备读函数声明*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *fpos);

/*设备写函数声明*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size,loff_t *fpos);
static void device_register_release(struct device *dev);       //逻辑设备释放处理函数

/*定义设备驱动文件结构体*/
struct file_operations mem_fops =
{
    .owner=THIS_MODULE,            //设备文件拥有者
    .open = mem_open,              //打开设备函数
    .release = mem_release,        //关闭设备函数
    .read = mem_read,              //读设备数据函数
    .write = mem_write,            //写设备数据函数
};

int __init device_initialize_add_del_init(void)
{
    int res;
    printk("into device_initialize_add_del_init\n");
    mem_spvm = (char *)vmalloc(MEM_MALLOC_SIZE);                      //开辟内存缓冲区
    res=register_chrdev(MEM_MAJOR, "my_char_dev", &mem_fops);          //注册字符设备
    if(res)                                                            //注册失败
    {
        unregister_chrdev(MEM_MAJOR, "my_char_dev");                   //删除字符设备
        printk("register char dev failed\n");
        return -1;
    }
    printk("register char dev success\n");
    mem_class = class_create(THIS_MODULE, "my_char_dev");              //创建设备类
    if(IS_ERR(mem_class))                                              //判断创建是否成功
    {
            printk("failed in creating class.\n");
            class_destroy(mem_class);                                  //失败，则销毁设备类
            return -1;
    }
    printk("class create success\n");
    dev=kzalloc(sizeof(*dev), GFP_KERNEL);               //为逻辑设备开辟内存空间
    device_initialize(dev);                               //初始化新创建的逻辑设备
    printk("device initialize success\n");
    dev->devt=MKDEV(MEM_MAJOR, MEM_MINOR);                //初始化逻辑设备设备号
    dev->class=mem_class;                                 //初始化设备类
    dev->release = device_register_release;               //初始化设备注销处理函数
    res= kobject_set_name(&dev->kobj, "my_char_dev"); //初始化设备引用计数器
    if(res)
    {
        printk("kobject_set_name_vargs failed\n");
        kfree(dev);                                       //失败，释放逻辑设备占用的内存空间
        return -1;
    }
    res=device_add(dev);       //将新创建的逻辑设备加入Linux内核系统
    if(res)
    {
        printk("device add failed\n");
        device_del(dev);                                  //失败，则删除逻辑设备
        kfree(dev);                                       //失败，释放逻辑设备的内存空间
        return -1;
    }
    printk("add device success\n");
    printk("out device_initialize_add_del_init\n");
    return 0;
}

void __exit device_initialize_add_del_exit (void)
{
    printk("into device_initialize_add_del_exit\n");
    unregister_chrdev(MEM_MAJOR, "my_char_dev");     //注销字符设备
    if(dev)
    {
        device_del(dev);         //删除逻辑设备
    }
    printk("device del success\n");
    if(mem_class)
        class_destroy(mem_class);      //删除设备类
    printk("class destroy success\n");
    if (mem_spvm != NULL)
        vfree(mem_spvm);                 //释放内存缓冲区空间
    printk("vfree mem_spvm OK\n");
    printk("out device_initialize_add_del_exit\n");
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
    if (size > MEM_MALLOC_SIZE)                    //判断读取数据的大小
        size = MEM_MALLOC_SIZE;
    if (tmp != NULL)
        res = copy_to_user(buf, tmp, size);        //将内核输入写入用户空间
    if (res == 0)
    {
        printk("copy data success and the data is:%s\n", tmp);     //显示读取的数据
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
        printk("read data success and the data is:%s\n", tmp);     //显示写入的数据
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
/*逻辑设备释放处理函数*/
static void device_register_release(struct device *dev)
{
    pr_debug("device: '%s': %s\n", dev_name(dev), __func__);
    kfree(dev);                                      //释放设备占用的内存空间
}

module_init(device_initialize_add_del_init);    //模块加载
module_exit(device_initialize_add_del_exit);    //模块卸载

