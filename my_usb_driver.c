#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

dev_t dev=0;

static int __init my_module_init(void){
	printk(KERN_INFO "Kernel module inserted\n");
	if((alloc_chrdev_region(&dev, 0,1,"John usb driver"))<0){
		printk(KERN_INFO "cannot allocate major num for device 1\n");
		return -1;
	}

	printk(KERN_INFO "Major:%d Minor: %d\n", MAJOR(dev),MINOR(dev));
	return 0;

}


static void  __exit my_module_exit(void){
	printk(KERN_INFO "kernel module exited\n");
	unregister_chrdev_region(dev,1);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOHN");
MODULE_VERSION("1.1");
MODULE_DESCRIPTION("dynamically allocated major num");
