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



#define mem_size 1024 // memory size
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

static int32_t value=0;
static char proc_array[20]="trying proc fs";
static int len = 1;  //length for proc
//proc parent name
static struct proc_dir_entry *parent;


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
static long int usb_ioctl(struct file *file,unsigned int cmd,unsigned long arg);

// fops related structure 
static struct file_operations fops=
{
	.owner=THIS_MODULE,
	.read=usb_read,
	.write=usb_write,
	.open=usb_open,
	.release= usb_release,
	.unlocked_ioctl=usb_ioctl,
};



/***************** Procfs Functions *******************/
static int      open_proc(struct inode *inode, struct file *file);
static int      release_proc(struct inode *inode, struct file *file);
static ssize_t  read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t  write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);

//proc fs ops structure
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
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
	if(copy_to_user(buf,kernel_buffer,mem_size)){
		pr_err("USB: data read :ERR\n");
	}
	pr_info("USB: data read done......\n");
	return len;
}

static ssize_t usb_write(struct file *filp,const char __user *buf,size_t len,loff_t *off){
	if(copy_from_user(kernel_buffer,buf,mem_size)){
		pr_err("USB: Data write : ERR\n");
	 }	 
	pr_info("USB: data write done!!!!....\n");
	pr_info("USB: data from user is: %s\n",kernel_buffer);
	return len;
}

/*
** This function will be called when we open the procfs file
*/
static int open_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file opend.....\t");
    return 0;
}
/*
** This function will be called when we close the procfs file
*/
static int release_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file released.....\n");
    return 0;
}

/*
** This function will be called when we read the procfs file
*/
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{
    pr_info("proc file read.....\n");
    if(len)
    {
        len=0;
    }
    else
    {
        len=1;
        return 0;
    }
    
    if( copy_to_user(buffer,proc_array,20) )
    {
        pr_err("Data Send : Err!\n");
	return -EFAULT;
    }
 
    return length;;
}

static long  int usb_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
//	pr_info("USB: arg address received : %lx",arg);


	switch(cmd){
		case WR_VALUE: if(copy_from_user(&value,(int32_t __user * )arg,sizeof(value))){
					pr_err("USB: Data wrtte : ERR\n");
					return -EFAULT;
			       }
			       pr_info("USB: value got from user to write :%d\n",value);
			       break;
		case RD_VALUE: if(copy_to_user((int32_t __user *)arg,&value,sizeof(value))){
					pr_err("USB: data read: ERR\n");
					return -EFAULT;
			       }
			       //pr_info("USB: data sendng to user: %d\n",value);
			       break;
		default: pr_info("USB: default cmd\n");
			      break;
	}
	return 0;
}

/*
** This function will be called when we write the procfs file
*/
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    pr_info("proc file wrote.....\n");
    
    if( copy_from_user(proc_array,buff,len) )
    {
        pr_err("Data Write : Err!\n");
    }
    
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
 /*Create proc directory. It will create a directory under "/proc" */
        parent = proc_mkdir("john_usb",NULL);
        if( parent == NULL )
        {
            pr_info("Error creating proc entry");
            goto r_device;
        }
        
        /*Creating Proc entry under "/proc/etx/" */
        proc_create("john_proc", 0666, parent, &proc_fops);
 
	//creating kernel memory at init
	if((kernel_buffer=kmalloc(mem_size,GFP_KERNEL))==0){
		pr_err("USB: cannot allocate memory....\n");
		goto r_device;
	}
	strcpy(kernel_buffer,"Hello World\n");

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
/* remove complete /proc/etx */
        proc_remove(parent);

	kfree(kernel_buffer);
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
MODULE_VERSION("1.6");
MODULE_DESCRIPTION("proc fs  setup to establish comm bw user and kernel space");
