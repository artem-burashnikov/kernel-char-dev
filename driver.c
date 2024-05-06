#include <linux/atomic.h>      /* Atomic operations usable in machine independent code */
#include <linux/cdev.h>        /* Character device manipulation */
#include <linux/fs.h>          /* Definitions for file table structures */
#include <linux/init.h>        /* Macros used to mark some functions or initialize data */
#include <linux/module.h>      /* Required by all modules */
#include <linux/moduleparam.h> /* Module parameters */
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

#define SUCCESS 0

#define MAX_LENGTH 80

static size_t crs_ord = 0;
module_param(crs_ord, ulong, 0);
MODULE_PARM_DESC(crs_ord, "Order of the CRS");

static uint8_t crs_const = 0;
module_param(crs_const, byte, 0);
MODULE_PARM_DESC(crs_const, "CRS constant");

static uint8_t crs_coeffs[MAX_LENGTH];
module_param_array(crs_coeffs, byte, NULL, 0);
MODULE_PARM_DESC(crs_coeffs, "An array of CRS coefficients");

static uint8_t crs_vals[MAX_LENGTH];
module_param_array(crs_vals, byte, NULL, 0);
MODULE_PARM_DESC(crs_vals, "An array of initial CRS bytes");

static GF_elem_t *crs_gf_const = NULL;
static GF_elem_t *crs_gf_coeffs[MAX_LENGTH];
static GF_elem_t *crs_gf_vals[MAX_LENGTH];
static GF_elem_t *acc_sum = NULL;
static GF_elem_t *prod = NULL;

static int rngdrv_open(struct inode *inode, struct file *file);
static int rngdrv_release(struct inode *inode, struct file *file);
static ssize_t rngdrv_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset);
static ssize_t rngdrv_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset);

enum {
        CDEV_NOT_USED = 0,
        CDEV_EXCLUSIVE_OPEN = 1,
};

static atomic_t cdev_status = ATOMIC_INIT(CDEV_NOT_USED);

static int major;
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
        if (atomic_cmpxchg(&cdev_status, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) {
                return -EBUSY;
        }

        pr_info("Successfully opened a device\n");
        try_module_get(THIS_MODULE);
  
        return SUCCESS;
}

static int rngdrv_release(struct inode *inode, struct file *file)
{
        atomic_set(&cdev_status, CDEV_NOT_USED);

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
        GF_elem_t *next_val;
        size_t i;
        uint8_t retval;
        
        retval = 0;

        next_val = GF_elem_cpy(crs_gf_const);
        
        for (i = 0; i < crs_ord; i++) {
                GF_elem_prod(prod, crs_gf_coeffs[i], crs_gf_vals[i]);
                GF_elem_sum(acc_sum, acc_sum, prod);
                memset(prod->poly->coeff, 0, prod->GF->I->deg * sizeof(prod->poly->coeff[0]));
                prod->poly->deg = 0;
        }
        
        GF_elem_sum(next_val, next_val, acc_sum);

        memset(acc_sum->poly->coeff, 0, acc_sum->GF->I->deg * sizeof(acc_sum->poly->coeff[0]));
        acc_sum->poly->deg = 0;

        GF_elem_destroy(crs_gf_vals[0]);

        memmove(crs_gf_vals, crs_gf_vals + 1, (crs_ord - 1) * sizeof(GF_elem_t *));

        crs_gf_vals[crs_ord - 1] = next_val;

        retval = GF_elem_to_uint8(next_val);
        
        if (copy_to_user(buffer, &retval, sizeof(retval))) {
                pr_err("Error copying data to user.\n");
        }

        return 1;
}

static int __init rngdrv_init(void)
{
        size_t i;
        
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
        acc_sum = GF_elem_get_neutral(&GF2_8);
        prod = GF_elem_get_neutral(&GF2_8);

        for (i = 0; i < crs_ord; ++i) {
                crs_gf_coeffs[i] = GF_elem_from_uint8(crs_coeffs[i]);
                crs_gf_vals[i] = GF_elem_from_uint8(crs_vals[i]);
        }

        return SUCCESS;
}

static void __exit rngdrv_cleanup(void)
{       
        size_t i;

        /* Destroy elements of CRS. */
        GF_elem_destroy(crs_gf_const);
        GF_elem_destroy(acc_sum);
        GF_elem_destroy(prod);

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
