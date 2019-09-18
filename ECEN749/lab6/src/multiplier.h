/* All of our linux kernel includes. */
#include <linux/module.h>  /* Needed by all modules */
#include <linux/moduleparam.h>  /* Needed for module parameters */
#include <linux/kernel.h>  /* Needed for printk and KERN_* */
#include <linux/init.h>	   /* Need for __init macros  */
#include <linux/fs.h>	   /* Provides file ops structure */
#include <linux/sched.h>   /* Provides access to the "current" process
			      task structure */
#include <linux/ioport.h>   // Used for io memory allocation
#include <linux/slab.h>
#include <asm/uaccess.h>   /* Provides utilities to bring user space
			      data into kernel space.  Note, it is
			      processor arch specific. */
#include <asm/io.h>         // Needed for IO reads and writes
#include "xparameters.h"    // Needed for IO reads and writes
#include "xparameters_ps.h"    // Needed for IO reads and writes



/* Some defines */
#define DEVICE_NAME "multiplier"

// From xparameters.h, physical address of multiplier
#define PHY_ADDR XPAR_MULTIPLY_0_S00_AXI_BASEADDR 
// Size of physical address range for multiply
#define MEMSIZE XPAR_MULTIPLY_0_S00_AXI_HIGHADDR - XPAR_MULTIPLY_0_S00_AXI_BASEADDR + 1


// Integer Buffer length
#define INT_BUF_LEN 3
/* Function prototypes, so we can setup the function pointers for dev
   file access correctly. */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

/* 
 * Global variables are declared as static, so are global but only
 * accessible within the file.
 */
static int Major;		/* Major number assigned to our device
				   driver */

static void* virt_addr;
static int* bf_Ptr;
static char* char_bf_Ptr; 
/* This structure defines the function pointers to our functions for
   opening, closing, reading and writing the device file.  There are
   lots of other pointers in this structure which we are not using,
   see the whole definition in linux/fs.h */
static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
};

