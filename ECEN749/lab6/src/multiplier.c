/* Moved all prototypes and includes into the headerfile */
#include "multiplier.h"


/*
 * This function is called when the module is loaded and registers a
 * device for the driver to use.
 */
int my_init(void) {

    virt_addr = (void*)ioremap(PHY_ADDR, MEMSIZE); // map physical address with virtual address
    bf_Ptr = (int*)kmalloc(INT_BUF_LEN*sizeof(int), GFP_KERNEL); // allocate the buffer for reading/writing data
    char_bf_Ptr = (char*)bf_Ptr; // cast the pointer to char type
    /* This function call registers a device and returns a major number
     associated with it.  Be wary, the device file could be accessed
     as soon as you register it, make sure anything you need (ie
     buffers ect) are setup _BEFORE_ you register the device.*/
    Major = register_chrdev(0, DEVICE_NAME, &fops); // register device
    
      /* Negative values indicate a problem */
    if (Major < 0) { // handle error
        printk(KERN_ALERT "Registering multiply device failed with %d\n", Major);
        return Major;
    }
    
    printk(KERN_INFO "Registered a device with dynamic Major number of %d\n", Major);
    
    printk(KERN_INFO "Create a device file for this device with this command: \n'mknod /dev/%s c %d 0'. \n", DEVICE_NAME, Major);
    
    return 0; /* success */
}

/*
 * This function is called when the module is unloaded, it releases
 * the device file.
 */
void my_cleanup(void)
{
	/* 
	 * Unregister the device 
	 */
	//print out a message for cleaning up
	printk(KERN_ALERT "unmapping virtual address space...\n");
	unregister_chrdev(Major, DEVICE_NAME); // unregister the char device
	iounmap((void*)virt_addr); // unmap the virtual aaddress
	kfree(bf_Ptr); // free the buffer memory

}

/*
 * Do nothing except print to the kernel message buffer informing 
 * the user when the device is opened. 
 */
static int device_open(struct inode *inode, struct file *file) {
	// print out a message when multiplier is opened
    printk(KERN_INFO "Multiplier is opened\n");
    return 0;
}

/* 
 * Do nothing except print to the kernel message buffer informing 
 * the user when the device is closed. 
 */
static int device_release(struct inode *inode, struct file *file) {
	// print out a message when multiplier is closed
    printk(KERN_INFO "Multiplier is closed\n");
    return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts
 * to read from it.
 */
static ssize_t 
device_read(struct file *filp, /* see include/linux/fs.h*/
		    char *buffer,      /* buffer to fill with data */
			size_t length,     /* length of the buffer  */
			loff_t * offset)
{
    int i, bytes_read; // declear increament and byte read varible
    /*
     * Valid value for length parameter include 0 through 12.
     */
    // printk(KERN_INFO "Reading multiplier...");
    if (length > 12){ // doesn't allow reading more than 12 bytes
        printk(KERN_ALERT "Sorry, reading more than 12 bytes isn't supported.\n");
        return -EINVAL;
    }
    
    /*
     * Number of bytes actually written to the buffer
     */
    //int bytes_read = 0;

    for (i = 0; i < 3; i++) // read three bytes from multiplier
        bf_Ptr[i] = ioread32(virt_addr + i*sizeof(int));

    //printk(KERN_INFO "READ: bf_Ptr[0] = %d, bf_Ptr[1] = %d, bf_Ptr[2] = %d\n", bf_Ptr[0], bf_Ptr[1], bf_Ptr[2]);
    char_bf_Ptr = (char*)bf_Ptr; // cast the buffer pointer to char type

    
    /* 
     * Actually put the data into the buffer
     */
    bytes_read = 0; // start from zero
    while (bytes_read < 12) { // read 12 bytes

        /* 
         * The buffer is in the user data segment, not the kernel segment
         * so "*" assignment won't work.  We have to use put_user which
         * copies data from the kernel data segment to the user data
         * segment.
         */
        //printk(KERN_INFO "put_user: char = %d\n", char_bf_Ptr[0]);
        put_user(*(char_bf_Ptr++), buffer++); /* one char at a time... */

        bytes_read++; // increment
    }

    /* 
     * Most read functions return the number of bytes put into the
     * buffer
     */
    return bytes_read;
}


static ssize_t
device_write(struct file *filp, 
             const char *buffer, 
             size_t length, 
             loff_t * offset)
{
    int i; // declearation for incrementor
    // printk(KERN_INFO "Writing multiplier...");
    if (length > 8){ // doesn't support more than 8 bytes 
        printk(KERN_ALERT "Sorry, writing more than 8 bytes isn't supported.\n");
        return -EINVAL; // throw an error message
    }
    
    for (i = 0; i < length; i++) // 12 chars
        get_user(char_bf_Ptr[i], buffer++); // write to kernel
    bf_Ptr = (int*) char_bf_Ptr; // cast to int type 
    
    for (i = 0; i < 2; i++) // 3 ints
        iowrite32(bf_Ptr[i], virt_addr + i * sizeof(int)); // write to register
    
    //printk(KERN_INFO "WRITE: bf_Ptr[0] = %d, bf_Ptr[1] = %d\n", bf_Ptr[0], bf_Ptr[1]);
    
    /* 
     * Return the number of input characters used 
     */
    return i;
}


/* These define info that can be displayed by modinfo */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tong Lu (and others)");
MODULE_DESCRIPTION("Module which creates a character device and allows user interaction with it");

/* Here we define which functions we want to use for initialization
   and cleanup */
module_init(my_init);
module_exit(my_cleanup);
