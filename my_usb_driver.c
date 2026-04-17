#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include<linux/err.h>

dev_t dev=0;
static struct class *dev_class;

static int __init my_module_init(void){
	if((alloc_chrdev_region(&dev, 0,1,"John_usb_driver"))<0){
		pr_err("cannot allocate major num for device 1\n");
		return -1;
	}
	pr_info( "Major:%d Minor: %d\n", MAJOR(dev),MINOR(dev));

	dev_class = class_create("USB_class");
	if(IS_ERR(dev_class)){
		pr_err("cannot create the struct class for device\n");
		goto r_class;
	}

	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"john_usb_device"))){
		pr_err("cannot create device\n");
		goto r_device;
	}
	printk(KERN_INFO "Kernel module inserted\n");
	
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;

}


static void  __exit my_module_exit(void){
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev,1);
	
	pr_info("kernel module exited\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOHN");
MODULE_VERSION("1.2");
MODULE_DESCRIPTION("creating device file automatically using class ");
