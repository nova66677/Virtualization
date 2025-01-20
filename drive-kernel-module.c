#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

/* IOCTL commands */
#define MY_RNG_IOCTL_RAND _IOR('q', 1, unsigned int) /* Get a random number */
#define MY_RNG_IOCTL_SEED _IOW('q', 1, unsigned int) /* Seed the RNG */

/* Physical base address of the device (replace with actual address) */
#define DEVICE_BASE_PHYS_ADDR 0x00ff

/* Memory-mapped I/O region pointer */
static void __iomem *devmem;

/* IOCTL handler function */
static long my_rng_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned int value;

    switch (cmd) {
    case MY_RNG_IOCTL_RAND:
        /* Generate a random number */
        value = ioread32(devmem); /* Read a value from the MMIO region */
        if (copy_to_user((unsigned int __user *)arg, &value, sizeof(value))) {
            return -EFAULT; /* Error copying to user space */
        }
        break;

    case MY_RNG_IOCTL_SEED:
        /* Seed the random number generator */
        if (copy_from_user(&value, (unsigned int __user *)arg, sizeof(value))) {
            return -EFAULT; /* Error copying from user space */
        }
        iowrite32(value, devmem); /* Write the seed value to the MMIO region */
        break;

    default:
        return -ENOTTY; /* Invalid command */
    }

    return 0; /* Success */
}

/* File operations structure */
static struct file_operations my_rng_fops = {
    .unlocked_ioctl = my_rng_ioctl,
    .owner = THIS_MODULE,
};

/* Module initialization function */
static int __init my_rng_driver_init(void)
{
    /* Map the device's MMIO region */
    devmem = ioremap(DEVICE_BASE_PHYS_ADDR, 4096);
    if (!devmem) {
        printk(KERN_ERR "Failed to map device registers in memory\n");
        return -ENOMEM;
    }

    /* Register the character device */
    if (register_chrdev(250, "my_rng_driver", &my_rng_fops) < 0) {
        printk(KERN_ERR "Failed to register my_rng_driver\n");
        iounmap(devmem);
        return -EBUSY;
    }

    printk(KERN_INFO "my_rng_driver loaded\n");
    printk(KERN_INFO "Registered IOCTLs: 0x%lx (get random number) and 0x%lx (seed generator)\n",
           MY_RNG_IOCTL_RAND, MY_RNG_IOCTL_SEED);
    return 0;
}

/* Module cleanup function */
static void __exit my_rng_driver_exit(void)
{
    /* Unregister the character device */
    unregister_chrdev(250, "my_rng_driver");

    /* Unmap the MMIO region */
    if (devmem) {
        iounmap(devmem);
    }

    printk(KERN_INFO "my_rng_driver unloaded\n");
}

/* Initialisation et cleanup des registres du module */
module_init(my_rng_driver_init);
module_exit(my_rng_driver_exit);

/* Metadata du module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eliott Chopin");
MODULE_DESCRIPTION("A simple RNG driver with IOCTL support");
MODULE_VERSION("1.0");
