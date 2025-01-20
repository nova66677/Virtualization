#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#define MY_RNG_IOCTL_RAND _IOR('q', 1, unsigned int)
#define MY_RNG_IOCTL_SEED _IOW('q', 1, unsigned int)
#define DEVICE_BASE_PHYS_ADDR 0xfebf1000

void *devmem = 0x0;
static long my_rng_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    unsigned int value;

    switch (cmd) {

    case MY_RNG_IOCTL_RAND:
        /* Generate a random number */
        value = ioread32(devmem); // Read a random value from the MMIO region
        if (copy_to_user((unsigned int __user *)arg, &value, sizeof(value))) {
            return -EFAULT; // Return error if unable to copy to user space
        }
        break;

    case MY_RNG_IOCTL_SEED:
        /* Seed the random number generator */
        if (copy_from_user(&value, (unsigned int __user *)arg, sizeof(value))) {
            return -EFAULT; // Return error if unable to copy from user space
        }
        iowrite32(value, devmem); // Write the seed value to the MMIO region
        break;

    default:
        return -ENOTTY; // Unknown command
    }

    return 0; // Success
}


static struct file_operations my_rng_fops = {
    .unlocked_ioctl = my_rng_ioctl,
};
static int __init my_rng_driver_init(void) {
    devmem = ioremap(DEVICE_BASE_PHYS_ADDR, 4096);

    if(!devmem) {
        printk(KERN_ERR "Failed to map device registers in memory");
        return -1;
    }

    if (register_chrdev(250, "my_rng_driver", &my_rng_fops) < 0) {
        printk(KERN_ERR "Failed to register my_rng_driver\n");
        return -1;
    }

    printk("my_rng_driver loaded, registered ioctls 0x%lx (get a random "
        "number) and 0x%lx (seed the generator) \n", MY_RNG_IOCTL_RAND,
        MY_RNG_IOCTL_SEED);
    return 0;
}

static void __exit my_rng_driver_exit(void) {
    unregister_chrdev(250, "my_rng_driver");

    if(devmem)
        iounmap(devmem);

    printk(KERN_INFO "my_rng_driver unloaded\n");
}

module_init(my_rng_driver_init);
module_exit(my_rng_driver_exit);
