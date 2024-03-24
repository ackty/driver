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
#include <linux/kobject.h>
MODULE_LICENSE("GPL");
#define MEM_MALLOC_SIZE    4096                       //缓冲区大小
#define MEM_MAJOR       245                           //主设备号
#define MEM_MINOR          0                          //次设备号
char *mem_spvm;                                      //缓冲区指针，指向内存区
struct class *mem_class;                             //设备类指针
struct device *dev;                                  //逻辑设备指针
static int  __init  get_put_device_init(void);        //模块加载函数声明
static void  __exit get_put_device_exit(void);        //模块卸载函数声明
static int mem_open(struct inode *ind, struct file *filp);     //设备打开函数声明
static int mem_release(struct inode *ind, struct file *filp); //设备关闭函数声明

/*设备读、写函数声明*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *fpos);
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size,loff_t *fpos);
static void device_register_release(struct device *dev);      //逻辑设备释放处理函数

/*定义设备驱动文件结构体*/
struct file_operations mem_fops =
{
    .owner=THIS_MODULE,                               //结构体拥有者初始化
    .open = mem_open,                                 //设备打开函数
    .release = mem_release,                           //设备关闭函数
    .read = mem_read,                                 //设备读函数
    .write = mem_write,                               //设备写函数
};

int __init get_put_device_init(void)
{
    int res;
    printk("into get_put_device_init\n");
    mem_spvm = (char *)vmalloc(MEM_MALLOC_SIZE);                    //开辟内存缓冲区
    res=register_chrdev(MEM_MAJOR, "my_char_dev", &mem_fops); //注册字符设备
    if(res)                                                          //注册失败
    {
        unregister_chrdev(MEM_MAJOR, "my_char_dev");                 //删除字符设备
        printk("register char dev failed\n");
        return -1;
    }
    printk("register char dev success\n");
    mem_class = class_create(THIS_MODULE, "my_char_dev");            //创建设备类
    if(IS_ERR(mem_class))                                            //判断创建是否成功
    {
            printk("failed in creating class.\n");
            class_destroy(mem_class);                                //失败，则销毁设备类
            return -1;
    }
    printk("class create success\n");
    dev=kzalloc(sizeof(*dev), GFP_KERNEL);                  //为逻辑设备开辟内存空间

    /*初始化设备的设备号，包括主设备号和次设备号*/
    dev->devt=MKDEV(MEM_MAJOR, MEM_MINOR);
    dev->class=mem_class;                                    //初始化设备类
    dev->release = device_register_release;                  //初始化设备注销处理函数
    res= kobject_set_name(&dev->kobj, "my_char_dev");        //初始化设备引用计数器
    if(res)
    {
        printk("kobject_set_name_vargs failed\n");
        kfree(dev);                                          //失败，释放设备占用的内存空间
        return -1;
    }
    res=device_register(dev);                         //将逻辑设备加入Linux内核系统
    if(res)
    {
        printk("register device failed\n");
        device_unregister(dev);                       //注册失败，释放逻辑设备
        kfree(dev);                                   //释放逻辑设备占用的内存空间
        return -1;
    }
    printk("register device success\n");

    /*显示当前设备的引用计数*/
    printk("the reference count of the device is:%d\n", dev->kobj.kref.refcount);
	get_device(dev);                                          //增加设备的引用计数
    printk("after the get_device the reference count of the device is:%d\n", dev->kobj.kref.refcount);                                      //显示函数调用之后设备的引用计数
    put_device(dev);                                  //减少设备的引用计数
    printk("after the put_device the reference count of the device is:%d\n", dev->kobj.kref.refcount);                                      //显示函数调用之后设备的引用计数
    printk("out get_put_device_init\n");
    return 0;
}

void __exit get_put_device_exit (void)
{
    printk("into get_put_device_exit\n");
    unregister_chrdev(MEM_MAJOR, "my_char_dev"); // 注销字符设备
    if(dev)
    {
        device_unregister(dev);                   //删除逻辑设备
    }
    printk("device unregister success\n");
    if(mem_class)
        class_destroy(mem_class);                 //删除设备类
    printk("class destroy success\n");
    if (mem_spvm != NULL)
        vfree(mem_spvm);                            //释放内存缓冲区空间
    printk("vfree mem_spvm OK\n");
    printk("out get_put_device_exit\n");
}

/*设备打开函数定义*/
int mem_open(struct inode *ind, struct file *filp)
{
    printk("open vmalloc space\n");
    try_module_get(THIS_MODULE);                  //模块引用自加
    printk("open vmalloc space success\n");
    return 0;
}

/*设备读函数定义，在此没有实际意义，因为不涉及设备的读*/
ssize_t mem_read(struct file *filp, char *buf, size_t size, loff_t *lofp)
{
    printk("in the function mem_read\n");
    return 0;
}

/*设备写函数定义，在此没有实际意义，因为不涉及设备的写*/
ssize_t mem_write(struct file *filp, const char *buf, size_t size, loff_t *lofp)
{
    printk("in the function mem_write\n");
    return 0;
}

/*设备关闭函数定义*/
int mem_release(struct inode *ind, struct file *filp)
{
    printk("close vmalloc space\n");
    module_put(THIS_MODULE);              //模块引用自减
    printk("close vmalloc space success\n");
    return 0;
}

/*逻辑设备释放处理函数*/
static void device_register_release(struct device *dev)
{
    pr_debug("device: '%s': %s\n", dev_name(dev), __func__);
    kfree(dev);                             //释放设备占用的内存空间
}

module_init(get_put_device_init);        //模块加载
module_exit(get_put_device_exit);        //模块卸载

