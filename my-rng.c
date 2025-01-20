#include "qemu/osdep.h"
#include "hw/pci/msi.h"
#include "hw/pci/pci.h"

#define TYPE_MY_RNG "my_rng"
#define MY_RNG(obj) OBJECT_CHECK(my_rng, (obj), TYPE_MY_RNG)

typedef struct {
    PCIDevice parent_obj;
    uint32_t seed_register;
    MemoryRegion mmio;
} my_rng;

static uint64_t mmio_read(void *opaque, hwaddr addr, unsigned size) {
    my_rng *dev = (my_rng *)opaque;

    switch (addr) {
    case 0x00: // Address 0x00: On souhaite recevoir une valeure au hasard
        return rand(); // On appelle rand() pour generer le nombre random
    case 0x04: // Address 0x04: On souhaite set une seed donnée
        return dev->seed_register;
    default:
        // Cas où l'on ne fait rien
        return 0x0;
    }
}

static void mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    my_rng *dev = (my_rng *)opaque;

    switch (addr) {
    case 0x00: 
        // Writing to 0x00 can trigger RNG behavior if needed (no-op in this case)
        break;
    case 0x04: // Address 0x04: Set the RNG seed
        dev->seed_register = (uint32_t)val;
        srand(dev->seed_register); // Seed the RNG
        break;
    default:
        // Invalid address
        return;
    }
}

static const MemoryRegionOps my_rng_ops = {
    .read = mmio_read,
    .write = mmio_write,
};

static void my_rng_realize(PCIDevice *pdev, Error **errp) {
    my_rng *s = MY_RNG(pdev);
    memory_region_init_io(&s->mmio, OBJECT(s), &my_rng_ops, s,
                          "my_rng", 4096);
    pci_register_bar(&s->parent_obj, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &s->mmio);
}

static void my_rng_class_init(ObjectClass *class, void *data) {
    DeviceClass *dc = DEVICE_CLASS(class);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(class);

    k->realize = my_rng_realize;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0xcafe;
    k->revision = 0x10;
    k->class_id = PCI_CLASS_OTHERS;
    
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static void my_rng_register_types(void) {
    static InterfaceInfo interfaces[] = {
        { INTERFACE_CONVENTIONAL_PCI_DEVICE },
        { },
    };

    static const TypeInfo my_rng_info = {
        .name = TYPE_MY_RNG,
        .parent = TYPE_PCI_DEVICE,
        .instance_size = sizeof(my_rng),
        .class_init    = my_rng_class_init,
        .interfaces = interfaces,
    };

    type_register_static(&my_rng_info);
}

type_init(my_rng_register_types)
