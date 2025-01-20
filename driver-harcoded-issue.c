#include <linux/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/ioport.h>  // Pour request_mem_region()

#define MY_RNG_IOCTL_RAND _IOR('q', 1, unsigned int)
#define MY_RNG_IOCTL_SEED _IOW('q', 1, unsigned int)

// Identifiants du vendeur et du périphérique
#define MY_RNG_VENDOR_ID 0x1234
#define MY_RNG_DEVICE_ID 0xcafe

static void __iomem *devmem = NULL;
static struct pci_dev *pdev = NULL;

static long my_rng_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    unsigned int value;

    switch (cmd) {
    case MY_RNG_IOCTL_RAND:
        value = ioread32(devmem);  // Lire une valeur aléatoire
        if (copy_to_user((unsigned int __user *)arg, &value, sizeof(value))) {
            return -EFAULT;  // Erreur de copie vers l'espace utilisateur
        }
        break;

    case MY_RNG_IOCTL_SEED:
        if (copy_from_user(&value, (unsigned int __user *)arg, sizeof(value))) {
            return -EFAULT;  // Erreur de copie depuis l'espace utilisateur
        }
        iowrite32(value, devmem);  // Écrire la graine dans la région MMIO
        break;

    default:
        return -ENOTTY;  // Commande inconnue
    }

    return 0;  // Succès
}

static struct file_operations my_rng_fops = {
    .unlocked_ioctl = my_rng_ioctl,
};

static int __init my_rng_driver_init(void) {
    struct resource *res;

    // Recherche du périphérique PCI avec le vendeur et l'ID de périphérique spécifiés
    pdev = pci_get_device(MY_RNG_VENDOR_ID, MY_RNG_DEVICE_ID, NULL);
    if (!pdev) {
        printk(KERN_ERR "Failed to find PCI device with vendor ID 0x%x and device ID 0x%x\n", MY_RNG_VENDOR_ID, MY_RNG_DEVICE_ID);
        return -ENODEV;
    }

    // Vérification que le périphérique a des ressources mémoire mappables
    res = &pdev->resource[0];  // L'adresse de base est généralement dans la première ressource
    if (!res || !res->start) {
        printk(KERN_ERR "PCI device does not have a valid memory region\n");
        pci_dev_put(pdev);
        return -ENODEV;
    }

    // Mappage de la région mémoire du périphérique
    devmem = ioremap(res->start, resource_size(res));
    if (!devmem) {
        printk(KERN_ERR "Failed to map device memory region\n");
        pci_dev_put(pdev);
        return -ENOMEM;
    }

    // Enregistrement du périphérique de caractère
    if (register_chrdev(250, "my_rng_driver", &my_rng_fops) < 0) {
        printk(KERN_ERR "Failed to register my_rng_driver\n");
        iounmap(devmem);
        pci_dev_put(pdev);
        return -1;
    }

    printk(KERN_INFO "my_rng_driver loaded, base address: %p\n", devmem);
    return 0;
}

static void __exit my_rng_driver_exit(void) {
    // Désenregistrement du périphérique de caractère et nettoyage
    unregister_chrdev(250, "my_rng_driver");
    if (devmem) {
        iounmap(devmem);
    }
    if (pdev) {
        pci_dev_put(pdev);
    }

    printk(KERN_INFO "my_rng_driver unloaded\n");
}

module_init(my_rng_driver_init);
module_exit(my_rng_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author Name");
MODULE_DESCRIPTION("Random Number Generator PCI Driver");
