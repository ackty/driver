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
#define MEM_MALLOC_SIZE 4096                                          //缓冲区大小
#define MEM_MAJOR 245 //主设备号，通过命令ll /dev查看系统已经存在的设备文件设备号
#define MEM_MINOR 0                                                   //次设备号
char *mem_spvm;                                                      //缓冲区指针，指向内存区
struct class *mem_class;                                             //设备类指针
static int  __init  class_register_unregister_init(void);             //模块加载函数声明
static void  __exit class_register_unregister_exit(void);             //模块卸载函数声明
static int mem_open(struct inode *ind, struct file *filp);          //设备打开函数声明
static int mem_release(struct inode *ind, struct file *filp); //设备关闭函数声明

/*设备读函数声明*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *fpos);

/*设备写函数声明*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size,loff_t *fpos);
static void class_create_release(struct class *cls);         //逻辑类释放处理函数声明

/*定义设备驱动文件结构体*/
struct file_operations mem_fops =
{
    .owner=THIS_MODULE,                                           //驱动文件拥有者
    .open = mem_open,                                             //设备打开函数
    .release = mem_release,                                       //设备释放函数
    .read = mem_read,                                             //设备读函数
    .write = mem_write,                                           //设备写函数
};

int __init class_register_unregister_init(void)
{
    int res;
    printk("into class_register_unregister_init\n");
    mem_spvm = (char *)vmalloc(MEM_MALLOC_SIZE);             //开辟内存缓冲区
    res=register_chrdev(MEM_MAJOR, "my_char_dev", &mem_fops); // 注册字符设备
    if(res)        //注册失败
    {
        unregister_chrdev(MEM_MAJOR, "my_char_dev");          //删除字符设备
        printk("register char dev failed\n");
        return -1;
    }
    printk("register char dev success\n");
    mem_class= kzalloc(sizeof(*mem_class), GFP_KERNEL);      //为设备类开辟内存空间
    if(IS_ERR(mem_class))                    //判断分配内存空间是否成功
    {
            kfree(mem_class);                //失败，释放开辟的内存空间
            printk("failed in kzalloc class.\n");
            return -1;
    }
    printk("kzalloc class success\n");
    mem_class->name ="my_char_dev";          //初始化设备类名
    mem_class->owner =THIS_MODULE;           //初始化设备类拥有者
    mem_class->class_release=class_create_release;           //初始化设备类释放处理函数
    int retval = class_register(mem_class);                  //注册设备类
    /*
    struct lock_class_key key;
    int retval = __class_register(mem_class, key);           //注册设备类
    */
    if(retval)                               //判断注册结果
    {
        kfree(mem_class);                    //失败，释放内存空间
        printk("failed in registing class\n");
        return -1;
    }
    printk("register class success\n");
    device_create(mem_class, NULL, MKDEV(MEM_MAJOR, MEM_MINOR), NULL, "my_char_dev");
                                             //注册设备文件系统，并建立设备节点
    printk("device create success\n");
    printk("out class_register_unregister_init\n");
    return 0;
}

void __exit class_register_unregister_exit(void)
{
    printk("into class_register_unregister_exit\n");
    unregister_chrdev(MEM_MAJOR, "my_char_dev");            //删除字符设备
    /*删除设备节点及目录*/
    device_destroy(mem_class, MKDEV(MEM_MAJOR, MEM_MINOR));
    if ((mem_class!= NULL)&&(! IS_ERR(mem_class)))
        class_unregister(mem_class);                        //删除设备类
    if (mem_spvm != NULL)
        vfree(mem_spvm);                                    //释放缓冲区空间
    printk("vfree ok! \n");
    printk("out class_register_unregister_exit\n");
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
        res = copy_to_user(buf, tmp, size);     //将内核输入写入用户空间
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
    if (size > MEM_MALLOC_SIZE)                   //判断输入数据的大小
        size = MEM_MALLOC_SIZE;
    if (tmp != NULL)
        res = copy_from_user(tmp, buf, size);   //将用户输入数据写入内核空间
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

/*设备类释放处理函数定义*/
void class_create_release(struct class *cls)
{
    pr_debug("%s called for %s\n", __func__, cls->name);
    kfree(cls);                                      //释放设备类的内存空间
}

module_init(class_register_unregister_init);    //模块加载
module_exit(class_register_unregister_exit);    //模块卸载

