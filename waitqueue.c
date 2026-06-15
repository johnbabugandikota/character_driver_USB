#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include<linux/err.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>     // kmalloc/kfree
#include<linux/uaccess.h>  // copy to/from user
#include<linux/ioctl.h>
#include<linux/proc_fs.h>
#include<linux/wait.h>
#include<linux/kthread.h>
#include<linux/workqueue.h>
#include<asm/io.h>
#include<linux/kobject.h>
#include<linux/interrupt.h>
#include<linux/sysfs.h>

//wait var
static uint32_t read_count=0;
static struct task_struct *wait_thread;
wait_queue_head_t wait_queue_usb;
static int wait_queue_flag=0;

//DECLARE_WAIT_QUEUE_HEAD(wait_queue_usb); static way of init wq

dev_t dev=0;
static struct class *dev_class;
static struct cdev usb_cdev;
uint8_t *kernel_buffer;

static int __init my_module_init(void);
static void  __exit my_module_exit(void);
static int usb_open(struct inode *inode,struct file *file);
static int usb_release(struct inode *inode, struct file *file);
static ssize_t usb_read(struct file *filp,char __user *buf,size_t len,loff_t* off);
static ssize_t usb_write(struct file *filp, const char *buf,size_t len,loff_t* off);
//static long int usb_ioctl(struct file *file,unsigned int cmd,unsigned long arg);


//thread func

static int wait_func(void* unused){
	while(1){
		pr_info("waiting for event\n");
		wait_event_interruptible(wait_queue_usb,wait_queue_flag!=0);
		if(wait_queue_flag==2){
			pr_info("event came from exit func\n");
			return 0;
		}
		pr_info("event came from read func-- %d\n",++read_count);
		wait_queue_flag=0;
	}
	return 0;
}

// fops related structure 
static struct file_operations fops=
{
	.owner=THIS_MODULE,
	.read=usb_read,
	.write=usb_write,
	.open=usb_open,
	.release= usb_release,
	
};



static int usb_open(struct inode *inode,struct file *file){
	pr_info("USB: Driver open called....\n");
	return 0;
}	

static int usb_release(struct inode *inode,struct file *file){
	pr_info("USB: driver release called.....\n");
	return 0;
}

static ssize_t usb_read(struct file *filp,char __user *buf,size_t len,loff_t *off ){
	pr_info("USB: data read done......\n");
	return len;
}

static ssize_t usb_write(struct file *filp,const char __user *buf,size_t len,loff_t *off){
		 
	pr_info("USB: data write called!!!!....\n");
	//pr_info("USB: data from user is: %s\n",kernel_buffer);
	return len;
}




static int __init my_module_init(void){
	//alloc major and minor num at run time
	if((alloc_chrdev_region(&dev, 0,1,"john_usb_driver"))<0){
		pr_err("USB: cannot allocate major num for device 1\n");
		return -1;
	}
	pr_info( "USB: Major:%d Minor: %d\n", MAJOR(dev),MINOR(dev));
	
	//init the cdev struct with fops 
	cdev_init(&usb_cdev,&fops);

	//adding the device to the system

	if ((cdev_add(&usb_cdev,dev,1))<0){
		pr_err("USB: cannot add the device to system...\n");
		goto r_class;
	}

	//creating struct class 
	dev_class = class_create("USB_class");
	if(IS_ERR(dev_class)){
		pr_err("USB: cannot create the struct class for device\n");
		goto r_class;
	}

	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"john_usb_device"))){
		pr_err("USB: cannot create device\n");
		goto r_device;
	}

	init_waitqueue_head(&wait_queue_usb); //init at run time
	wait_thread=kthread_create(wait_func,NULL,"waitThread");
	
	if(wait_thread){
		pr_info("thread created successfuly\n");
		wake_up_process(wait_thread);
	}
	else{
		pr_info("thread creation failed\n");
	} 

	pr_info("USB: Kernel module inserted........\n");
	
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;

}

//module exit func

static void  __exit my_module_exit(void){

	wait_queue_flag=2;
	wake_up_interruptible(&wait_queue_usb);

	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&usb_cdev);
	unregister_chrdev_region(dev,1);
	
	pr_info("USB: kernel module exited\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOHN");
MODULE_DESCRIPTION("wait queue implementation");
