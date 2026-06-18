#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/err.h>
#define IRQ_NO 1

//Interrupt handler for IRQ 1 
static irqreturn_t irq_handler(int irq,void *dev_id) {
  printk(KERN_INFO "Shared IRQ: Interrupt Occurred");
  return IRQ_HANDLED;
}


volatile int usb_value=0;

dev_t dev=0;
static struct class *dev_class;
static struct cdev usb_cdev;
struct kobject *kobj_ref;

/*
** Function Prototypes
*/
static int      __init usb_driver_init(void);
static void     __exit usb_driver_exit(void);

/*************** Driver functions **********************/
static int      usb_open(struct inode *inode, struct file *file);
static int     usb_release(struct inode *inode, struct file *file);
static ssize_t  usb_read(struct file *filp, 
                        char __user *buf, size_t len,loff_t * off);
static ssize_t  usb_write(struct file *filp, 
                        const char *buf, size_t len, loff_t * off);

/*************** Sysfs functions **********************/
static ssize_t  sysfs_show(struct kobject *kobj, 
                        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj, 
                        struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute usb_attr = __ATTR(usb_value, 0660, sysfs_show, sysfs_store);


/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = usb_read,
        .write          = usb_write,
        .open           = usb_open,
        .release        = usb_release,
};

/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
        pr_info("Sysfs - Read!!!\n");
        return sprintf(buf, "%d", usb_value);
}

/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject *kobj, 
                struct kobj_attribute *attr,const char *buf, size_t count)
{
        pr_info("Sysfs - Write!!!\n");
        sscanf(buf,"%d",&usb_value);
        return count;
}


/*
** This function will be called when we open the Device file
*/ 
static int usb_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/ 
static int usb_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
 
/*
** This function will be called when we read the Device file
*/
static ssize_t usb_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t usb_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}



/*
** Module Init function
*/
static int __init usb_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "john_usb")) <0){
                pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&usb_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&usb_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create("usb_class"))){
            pr_info("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"john_usb"))){
            pr_info("Cannot create the Device 1\n");
            goto r_device;
        }
 
        /*Creating a directory in /sys/kernel/ */
        kobj_ref = kobject_create_and_add("usb_sysfs",kernel_kobj);
 
        /*Creating sysfs file for etx_value*/
        if(sysfs_create_file(kobj_ref,&usb_attr.attr)){
                pr_err("Cannot create sysfs file......\n");
                goto r_sysfs;
    }
	if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "etx_device", (void *)(irq_handler))) {
            printk(KERN_INFO "my_device: cannot register IRQ ");
                    goto irq;
        }

        pr_info("Device Driver Insert...Done!!!\n");
        return 0;

irq:
	free_irq(IRQ_NO,(void*)irq_handler);
 
r_sysfs:
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &usb_attr.attr);
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&usb_cdev);
        return -1;
}
/*
** Module exit function
*/
static void __exit usb_driver_exit(void)
{
        free_irq(IRQ_NO,(void *)(irq_handler));	kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &usb_attr.attr);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&usb_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(usb_driver_init);
module_exit(usb_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John");
MODULE_DESCRIPTION("Basic interrupt implementation by detecting key press using interrupt");
