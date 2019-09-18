#include <linux/module.h>   // Needed by all modules
#include <linux/kernel.h>   // Needed for KERN_* and printk
#include <linux/init.h>     // Needed for __init and __exit macros
#include <asm/io.h>         // Needed for IO reads and writes
#include "xparameters.h"    // Needed for IO reads and writes
#include <linux/ioport.h>   // Used for io memory allocation

// From xparameters.h, physical address of multiplier
#define PHY_ADDR XPAR_MULTIPLY_0_S00_AXI_BASEADDR 
// Size of physical address range for multiply
#define MEMSIZE XPAR_MULTIPLY_0_S00_AXI_HIGHADDR - XPAR_MULTIPLY_0_S00_AXI_BASEADDR + 1

// virtual address pointing to multiplier
void* virt_addr; 

/* This function is run upon module load. This is where you setup data
   structures and reserve resources used by the module */
static int __init my_init(void)
{
    // Linux kernel's version of printf
    printk(KERN_INFO "Mapping virtual address...\n");
    
    // map virtual address to multiplier physical address
    // use ioremap, print the physical and virtual address 
    virt_addr = (void*)ioremap(PHY_ADDR, MEMSIZE);
    printk("Physical address: %x; Virtual address: %x\n.", (unsigned int)virt_addr, (unsigned int)PHY_ADDR); 
    // write 7 to register 0
    printk(KERN_INFO "Writing a 7 to register 0\n");
    iowrite32(7, virt_addr + 0);     // base address + offset
    // write 2 to register 1
    printk(KERN_INFO "Writing a 2 to register 1\n");
    // use iowrite32
    iowrite32(2, virt_addr + 4);
 
    printk("Read %d from register 0\n", ioread32(virt_addr+0));
    printk("Read %d from register 1\n", ioread32(virt_addr+4));
    printk("Read %d from register 2\n", ioread32(virt_addr+8));
    
    // A non 0 return means init_module failed; module can't be loaded
    return 0;
}

/* This function is run just prior to the module's removal from the system.
   You should release ALL resources used by your module here (otherwise be
   prepared for a reboot). */
static void __exit my_exit(void)
{
    printk(KERN_ALERT "unmapping virtual address space...\n");
    iounmap((void*)virt_addr);
}

// These define info that can be displayed by modinfo
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ECEN449 Student (and others)");
MODULE_DESCRIPTION("Simple multiplier module");

// Here we define which functions we want to use for initialization and cleanup
module_init(my_init);
module_exit(my_exit);
