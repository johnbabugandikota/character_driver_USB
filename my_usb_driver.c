#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>


static int __init my_module_init(void){
	printk(KERN_INFO "Kernel module inserted\n");
	return 0;

}


static void  __exit my_module_exit(void){
	printk(KERN_INFO "kernel module exited\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOHN");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("simple usb driver");
