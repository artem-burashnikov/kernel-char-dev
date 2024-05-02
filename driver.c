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

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "rngdrv"

enum {
  CDEV_NOT_USED = 0,
  CDEV_EXCLUSIVE_OPEN = 1,
};

static int major;

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

static struct class *cls;

static struct file_operations fops = {
  .owner = THIS_MODULE,
};

static int __init init_rngdrv(void) {
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

static void __exit cleanup_rngdrv(void) {
  device_destroy(cls, MKDEV(major, 0));
  class_destroy(cls);
  unregister_chrdev(major, DEVICE_NAME);
  pr_info("Successfully unregistered and destroyed a device\n");
  return;
} 

module_init(init_rngdrv);
module_exit(cleanup_rngdrv);
