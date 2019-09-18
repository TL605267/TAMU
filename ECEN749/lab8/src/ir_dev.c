/*  irq_test.c - Simple character device module
 *  
 *  Demonstrates interrupt driven character device.  Note: Assumption
 *  here is some hardware will strobe a given hard coded IRQ number
 *  (200 in this case).  This hardware is not implemented, hence reads
 *  will block forever, consider this a non-working example.  Could be
 *  tied to some device to make it work as expected.
 *
 *  (Adapted from various example modules including those found in the
 *  Linux Kernel Programming Guide, Linux Device Drivers book and
 *  FSM's device driver tutorial)
 */

/* Moved all prototypes and includes into the headerfile */
#include "ir_dev.h"
//reset; make ARCH=arm CROSS_COMPILE=arm-xilinx-linux-gnueabi-


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

/*
 * This function is called when the module is loaded and registers a
 * device for the driver to use.
 */
int my_init(void)
{
  virt_addr = (void*)ioremap(PHY_ADDR, MEMSIZE); // map virtual address to physical address
  init_waitqueue_head(&queue);	/* initialize the wait queue */

  /* Initialize the semaphor we will use to protect against multiple
     users opening the device  */
  sema_init(&sem, 1);

  Major = register_chrdev(0, DEVICE_NAME, &fops); // register character device 
  if (Major < 0) { // error handling
    printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  }
  printk(KERN_INFO "Registered a device with dynamic Major number of %d\n", Major); // print Major number
  printk(KERN_INFO "Create a device file for this device with this command:\n'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

  return 0;		/* success */
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
  printk(KERN_ALERT "unmapping virtual address space...\n");
  unregister_chrdev(Major, DEVICE_NAME); // unregister character device
  iounmap((void*)virt_addr); // unmap virtual address
}


/* 
 * Called when a process tries to open the device file, like "cat
 * /dev/irq_test".  Link to this function placed in file operations
 * structure for our device file.
 */
static int device_open(struct inode *inode, struct file *file)
{
  int irq_ret;
      
  if (down_interruptible (&sem))
	return -ERESTARTSYS;	

  /* We are only allowing one process to hold the device file open at
     a time. */
  if (Device_Open){
    up(&sem);
    return -EBUSY;
  }
  Device_Open++;
  
  /* OK we are now past the critical section, we can release the
     semaphore and all will be well */
  up(&sem);
  
  /* request a fast IRQ and set handler */
  irq_ret = request_irq(IRQ_NUM, irq_handler, 0 /*flags*/ , DEVICE_NAME, NULL);
  if (irq_ret < 0) {		/* handle errors */
    printk(KERN_ALERT "Registering IRQ failed with %d\n", irq_ret);
    return irq_ret;
  }

  try_module_get(THIS_MODULE);	/* increment the module use count
				   (make sure this is accurate or you
				   won't be able to remove the module
				   later. */

  msg_queue_Ptr = msg_queue_new(QUEUE_LEN); // allocate message queue
  return 0;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
  Device_Open--;		/* We're now ready for our next caller */
  
  free_irq(IRQ_NUM, NULL);
  msg_queue_destroy(msg_queue_Ptr); // free message queue
  /* 
   * Decrement the usage count, or else once you opened the file,
   * you'll never get get rid of the module.
   */
  module_put(THIS_MODULE);
  
  return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
  int bytes_read = 0;
  ir_msg temp_msg; // temporary message varible
  /* In this driver msg_Ptr is NULL until an interrupt occurs */
  //wait_event_interruptible(queue, (msg_Ptr != NULL)); /* sleep until interrupted */

  if (DEBUG) printk(KERN_INFO "device_read\n"); // print debug statement
  /*
   * If we're at the end of the message, 
   * return 0 signifying end of file 
   */
  /*if (*msg_Ptr == 0) {
    msg_Ptr = NULL;	// completed interrupt servicing reset pointer to wait for another interrupt 
    if (DEBUG) printk(KERN_INFO "device_read: completed interrupt servicing\n");
    return 0;
  }*/
  
  /* 
   * Actually put the data into the buffer 
   */
  while (length && dequeue(msg_queue_Ptr, &temp_msg) >= 0) {
    
    /* 
     * The buffer is in the user data segment, not the kernel 
     * segment so "*" assignment won't work.  We have to use 
     * put_user which copies data from the kernel data segment to
     * the user data segment. 
     */
     if (DEBUG) printk(KERN_INFO "Read: loop\n"); // debug statement
      put_user(temp_msg.byte_low, buffer++); /* one char at a time... */
      put_user(temp_msg.byte_high, buffer++); /* one char at a time... */
      length--; // decrement length
      bytes_read++; // increment bytes_read

  }
  
  /* 
   * Most read functions return the number of bytes put into the buffer
   */
  return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 * Next time we'll make this one do something interesting.
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{

  /* not allowing writes for now, just printing a message in the
     kernel logs. */
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;		/* Fail */
}

irqreturn_t irq_handler(int irq, void *dev_id) {
  static int counter = 0;	/* keep track of the number of
				   interrupts handled */
  int raw_data, raw_count; // temporary varible for data and count
  ir_msg* temp_msg; // temporary pointer for message
  
  //sprintf(msg, "IRQ Num %d called, interrupts processed %d times\n", irq, counter++);
  //msg_Ptr = msg;
  interrupt_counter += 1; // interrupt counter increment
  raw_data = ioread32(virt_addr); // read slv_reg0
  temp_msg = ir_msg_new((raw_data >> 8) & 0xff, raw_data & 0xff); // convert to ir_msg type
  enqueue(msg_queue_Ptr, temp_msg); // put message into msg_queue
  if (DEBUG) printk(KERN_INFO "irq_handler: raw_data = %x\n", raw_data); // debug statement
  printk(KERN_INFO "%dth interrupt: raw_data = %x\n", interrupt_counter, raw_data); // print out interrput
  iowrite16(1, virt_addr + 8); // reset interrput 
  while (ioread32(virt_addr + 8)&0x8000); // wait till interrupt
  iowrite16(0, virt_addr + 8); // clear reset bit
  
  wake_up_interruptible(&queue);   /* Just wake up anything waiting for the device */
  return IRQ_HANDLED;
}



/* These define info that can be displayed by modinfo */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul V. Gratz (and others)");
MODULE_DESCRIPTION("Module which creates a character device and allows user interaction with it");

/* Here we define which functions we want to use for initialization
   and cleanup */
module_init(my_init);
module_exit(my_cleanup);
