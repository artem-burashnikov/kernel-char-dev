#include <linux/atomic.h> /* Atomic operations usable in machine independent code */
#include <linux/cdev.h>   /* Character device manipulation */
#include <linux/fs.h>     /* Definitions for file table structures */
#include <linux/init.h>   /* Macros used to mark some functions or initialized data */
#include <linux/module.h> /* Required by all modules */
#include <linux/printk.h> /* For logging */
#include <linux/types.h>  /* Linux specific types */
#include <asm/uaccess.h>  /* User space memory access functions */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artem Burashnikov"); 
MODULE_DESCRIPTION("A pseudo-random number generator.");

static int rngdrv_open(struct inode *inode, struct file *file);
static int rngdrv_release(struct inode *inode, struct file *file);
static ssize_t rngdrv_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset);
static ssize_t rngdrv_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset);

#define SUCCESS 0
#define DEVICE_NAME "rngdrv"

enum {
        CDEV_NOT_USED = 0,
        CDEV_EXCLUSIVE_OPEN = 1,
};

static int major;

static atomic_t cdev_in_use = ATOMIC_INIT(CDEV_NOT_USED);

static struct class *cls;

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = rngdrv_open,
        .release = rngdrv_release,
        .write = rngdrv_write,
        .read = rngdrv_read,
};

static int rngdrv_open(struct inode *inode, struct file *file)
{
        if (atomic_cmpxchg(&cdev_in_use, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) return -EBUSY;
        
        pr_info("Successfully opened a device\n");
        try_module_get(THIS_MODULE);
  
        return SUCCESS;
}

static int rngdrv_release(struct inode *inode, struct file *file)
{
        atomic_set(&cdev_in_use, CDEV_NOT_USED);
        module_put(THIS_MODULE);
        pr_info("Successfully closed a device\n");
        return SUCCESS;
}

static ssize_t rngdrv_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset)
{
        pr_alert("Write operation is not supported.\n"); 
        return -EINVAL; 
}

static ssize_t rngdrv_read(struct file *file, char __user *buffer, size_t count, loff_t *offset)
{
        /* TODO */
        return SUCCESS;
}

static int __init rngdrv_init(void)
{
        major = register_chrdev(0, DEVICE_NAME, &fops);
        if (major < 0) {
                pr_alert("Failed to initialize a device with major %d\n", major);
                return major;
        }

        pr_info("Successfully initialized a device with major %d\n", major);

        cls = class_create(DEVICE_NAME);
        device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
        pr_info("Device is created at /dev/%s\n", DEVICE_NAME);

        return SUCCESS;
}

static void __exit rngdrv_cleanup(void)
{
        device_destroy(cls, MKDEV(major, 0));
        class_destroy(cls);
        unregister_chrdev(major, DEVICE_NAME);
        pr_info("Successfully unregistered and destroyed a device\n");
        return;
} 

module_init(rngdrv_init);
module_exit(rngdrv_cleanup);
