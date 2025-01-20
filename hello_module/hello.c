#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/init.h>   /* Needed for the macros */

/* Module initialization function */
static int __init hello_start(void)
{
    printk(KERN_INFO "Loading hello module...\n");
    printk(KERN_INFO "Hello, world!\n");
    return 0;
}

/* Module cleanup function */
static void __exit hello_end(void)
{
    printk(KERN_INFO "Unloading hello module...\n");
    printk(KERN_INFO "Goodbye, world!\n");
}

/* Register module initialization and cleanup functions */
module_init(hello_start);
module_exit(hello_end);

/* Module metadata */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World kernel module");
MODULE_VERSION("1.0");

