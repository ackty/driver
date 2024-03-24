/*头文件引用*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");
struct cdev *mem_cdev;            //字符设备对象

static int __init cdev_alloc_init (void)
{
    printk("into the cdev_alloc_init\n");
    mem_cdev = cdev_alloc();        //调用函数动态分配字符设备
    if (mem_cdev == NULL)            //检测函数调用成功与否
    {
        printk("cdev_alloc failed! \n");
        return -1;
    }

    /*显示设备地址空间*/
    printk("cdev_alloc success! addr = 0x%x\n", (unsigned int)mem_cdev); if(&(mem_cdev->list) != NULL)                      //检测函数调用结果，对list的初始化情况
        printk("the list_head of the mem_cdev has been initialized\n");
    if(&(mem_cdev->kobj) != NULL)     //检测函数调用结果，对字段kobj的初始化情况
    {
        printk("the kobj of the mem_cdev has been initialized\n");
        printk("the state_in_sysfs of the kobj of the mem_cdev is:%d\n", mem_cdev->kobj.state_in_sysfs);
        printk("the state_initialized of the kobj of the mem_cdev is:%d\n", mem_cdev->kobj.state_initialized);
    }
    printk("out the cdev_alloc_init\n");
    return 0;
}

static void __exit cdev_alloc_exit (void)
{
    printk("into cdev_alloc_exit\n");
    if (mem_cdev != NULL)
        kfree(mem_cdev);          //释放设备空间
    printk("kfree mem_cdev OK! \n");
    printk("out cdev_alloc_exit\n");
}

module_init(cdev_alloc_init);    //模块加载函数调用
module_exit(cdev_alloc_exit);    //模块卸载函数调用

