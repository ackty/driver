/*头文件引用*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
MODULE_LICENSE("GPL");

/*全局变量定义*/
struct class *mem_class;        //设备类指针

static int __init class_create_destroy_init (void)
{
    printk("into the class_create_destroy_init\n");
    mem_class = class_create(THIS_MODULE, "my_char_dev");      //创建设备类

/*
    struct lock_class_key key;
    mem_class=__class_create(THIS_MODULE, "my_char_dev", &key); //创建设备类
*/

    if(IS_ERR(mem_class))      //判断创建是否成功
        {
            printk("Err: failed in creating class.\n");
            return -1;
    }
    printk("class create success\n");
    printk("out the class_create_destroy_init\n");
    return 0;
}

void __exit class_create_destroy_exit (void)
{
    printk("into class_create_destroy _exit\n");
        class_destroy(mem_class);              //删除设备类
    printk("the mem_class has been destroyed\n");
    printk("out class_create_destroy_exit\n");
}

module_init(class_create_destroy_init);      //模块加载函数调用
module_exit(class_create_destroy_exit);      //模块卸载函数调用

