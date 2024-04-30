#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artem Burashnikov"); 
MODULE_DESCRIPTION("A pseudo-random number generator."); 

static int __init init_hello(void) { 
    return 0; 
} 

static void __exit cleanup_hello(void) {
    return;
} 


module_init(init_hello); 
module_exit(cleanup_hello);
