#include <linux/atomic.h>      /* Atomic operations usable in machine independent code */
#include <linux/cdev.h>        /* Character device manipulation */
#include <linux/fs.h>          /* Definitions for file table structures */
#include <linux/init.h>        /* Macros used to mark some functions or initialized data */
#include <linux/module.h>      /* Required by all modules */
#include <linux/moduleparam.h> /* Module parameters. */
#include <linux/printk.h>      /* For logging */
#include <linux/types.h>       /* Linux specific types */
#include <asm/uaccess.h>       /* User space memory access functions */

#include "GF.h"
#include "poly.h"
#include "utils.h"

#define DEVICE_NAME "rngdrv"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artem Burashnikov"); 
MODULE_DESCRIPTION("A pseudo-random number generator.");

/* Return status code. */
#define SUCCESS 0

/* MAX_LENGTH is set to 92 because ssize_t can't fit the number > 92 */
#define MAX_LENGTH 92

/* Order of the CRS. */
static uint8_t crs_ord = 0;
module_param(crs_ord, byte, 0000);
MODULE_PARM_DESC(crs_ord, "Order of the CRS");

/* Initial CRS constant. */
static uint8_t crs_const = 0;
module_param(crs_const, byte, 0000);
MODULE_PARM_DESC(crs_const, "CRS constant");

/* Array of initial coeffitions. */
static uint8_t crs_coeffs[MAX_LENGTH];
module_param_array(crs_coeffs, byte, NULL, 0000);
MODULE_PARM_DESC(crs_coeffs, "An array of CRS coefficients");

/* Array of inital values. */
static uint8_t crs_vals[MAX_LENGTH];
module_param_array(crs_vals, byte, NULL, 0000);
MODULE_PARM_DESC(crs_vals, "An array of initial CRS bytes");

/* CRS over finite field. */
static GF_elem_t *crs_gf_const = NULL;
static GF_elem_t *crs_gf_coeffs[MAX_LENGTH];
static GF_elem_t *crs_gf_vals[MAX_LENGTH];
static GF_elem_t *crs_gf_next_val = NULL;

/* File operations prototpyes. */
static int rngdrv_open(struct inode *inode, struct file *file);
static int rngdrv_release(struct inode *inode, struct file *file);
static ssize_t rngdrv_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset);
static ssize_t rngdrv_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset);

/* Device availability status. */
enum {
        CDEV_NOT_USED = 0,
        CDEV_EXCLUSIVE_OPEN = 1,
};

/* Used to prevent concurrent access into the same device. */ 
static atomic_t cdev_in_use = ATOMIC_INIT(CDEV_NOT_USED);

/* Character device stuff. */
static int major;
static struct class *cls;

/* File interface implementation. */
static struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = rngdrv_open,
        .release = rngdrv_release,
        .write = rngdrv_write,
        .read = rngdrv_read,
};

static int rngdrv_open(struct inode *inode, struct file *file)
{
        if (atomic_cmpxchg(&cdev_in_use, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) {
                return -EBUSY;
        }
        
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
        uint8_t i;
        
        /* Register and create the device dynamically. */
        major = register_chrdev(0, DEVICE_NAME, &fops);
        if (major < 0) {
                pr_alert("Failed to initialize a device with major %d\n", major);
                return major;
        }

        pr_info("Successfully initialized a device with major %d\n", major);

        cls = class_create(DEVICE_NAME);
        device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
        pr_info("Device is created at /dev/%s\n", DEVICE_NAME);

        /* Set initial elements of CRS. */
        crs_gf_const = GF_elem_from_uint8(crs_const);
        crs_gf_next_val = GF_elem_get_neutral(&GF2_8);
        for (i = 0; i < crs_ord; ++i) {
                crs_gf_coeffs[i] = GF_elem_from_uint8(crs_coeffs[i]); 
                crs_gf_vals[i] = GF_elem_from_uint8(crs_vals[i]);
        }

        return SUCCESS;
}

static void __exit rngdrv_cleanup(void)
{       
        uint8_t i;

        /* Free elements of CRS. */
        GF_elem_destroy(crs_gf_const);
        GF_elem_destroy(crs_gf_next_val);
        for (i = 0; i < crs_ord; ++i) {
                GF_elem_destroy(crs_gf_coeffs[i]);
                GF_elem_destroy(crs_gf_vals[i]);
        }

        device_destroy(cls, MKDEV(major, 0));
        class_destroy(cls);
        unregister_chrdev(major, DEVICE_NAME);
        pr_info("Successfully unregistered and destroyed a device\n");
        return;
} 

module_init(rngdrv_init);
module_exit(rngdrv_cleanup);
