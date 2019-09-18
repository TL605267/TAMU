#include <linux/slab.h>    /* Provide kmalloc()/kfree() */
// debug flag
#define DEBUG 0
#define QUEUE_LEN 100 // set the length of queue to 100
typedef struct ir_msg { // define a 16 bits data type to store message
    unsigned char byte_low; // lower bits
    unsigned char byte_high; // upper bits
} ir_msg;

//typedef unsigned short int ir_msg; // use 2-byte data type for message
ir_msg* ir_msg_new(unsigned char high, unsigned char low) { // initalize ir_message
    ir_msg* new_ir_msg = (ir_msg*)kmalloc(sizeof(ir_msg), GFP_KERNEL); // allocate message
    new_ir_msg -> byte_low = low; // assign lower bits
    new_ir_msg -> byte_high = high; // assign upper bits
    return new_ir_msg; // return pointer
}

typedef struct msg_queue { // define a message queue structure using circular array
    int head_idx, tail_idx, size, capacity; // parameters
    ir_msg* msg_array; // circular array pointer
} msg_queue;

msg_queue* msg_queue_new (int capacity) { // initalize msg_queue
    msg_queue* queue = (msg_queue*)kmalloc(sizeof(msg_queue), GFP_KERNEL); // allocate queue
    queue -> capacity = capacity; // assign capacity
    queue -> head_idx = 0; // initalize head index
    queue -> tail_idx = 0; // initalize tail index
    queue -> size = 0; // initalize queue size
    queue -> msg_array = (ir_msg*)kmalloc(QUEUE_LEN*sizeof(ir_msg), GFP_KERNEL); // allocate array
    return queue;
}

void msg_queue_destroy (msg_queue* queue) { // free queue
    if (queue == NULL) { // handle null pointer
        printk(KERN_INFO "Queue is NULL!\n");
        return;
    }
    else {
        kfree(queue -> msg_array);  // free array first
        kfree(queue); // then free queue
        return;
    }
}

int is_full (msg_queue* queue) { // check if the queue is full
    if (queue == NULL) { // handle null pointer
        printk(KERN_INFO "Queue is NULL!\n");
        return -2;
    }
    if (queue -> size == queue -> capacity) { // if queue size reach the capacity
        if (DEBUG) printk(KERN_INFO "is_full: queue is full"); // then queue is full
        return 1; // return 1 for full
    }
    else return 0; // 0: not full yet
}

int is_empty (msg_queue* queue) { // check if the queue is full
    if (queue == NULL) { // handle null pointer
        printk(KERN_INFO "Queue is NULL!\n");
        return -2;
    }
    if (queue -> size == 0) { // if queue size is zero
        if (DEBUG) printk(KERN_INFO "is_empty: queue is empty"); // then queue is empty
        return 1; // return 1 for empty
    }
    else return 0; // 0: not empty yet
}

void print_queue (msg_queue* queue) { // print function for debug
    if (queue == NULL) { // handle null pointer
        printk(KERN_INFO "Queue is NULL!\n");
        return;
    }
    if (is_empty(queue)) return; // if queue is empty then do nothing
    else { // if not empty
        int i; // for loop varible
        //printk(KERN_INFO "print_queue: ");
        if (queue -> head_idx <= queue -> tail_idx) { // if head index is less than tail index
            for (i = queue -> head_idx; i <= queue -> tail_idx; i++) { // then iterate from head to tail
                printk(KERN_INFO "%x%x, ", queue->msg_array[i].byte_high, queue->msg_array[i].byte_low); // print messages from array
            }
        }
        else { // if tail index is less than head index
            for (i = queue -> tail_idx; i <= queue -> head_idx; i++) { // then iterate from tail to head
                printk(KERN_INFO "%x%x, ", queue->msg_array[i].byte_high, queue->msg_array[i].byte_low); // print messages from array
            }
        }
        printk(KERN_INFO "\n"); // new line
        return;
    }
}

int enqueue (msg_queue* queue, ir_msg* msg_in) { // put message into queue
    if (queue == NULL) { // handle null pointer
        printk(KERN_INFO "Queue is NULL!\n");
        return -2;
    }
    if (is_full(queue)) return -1; // if full then don't enqueue
    else { // if not full yet
        queue -> tail_idx = (queue -> tail_idx + 1) % queue -> capacity; // tail index increment in circular manner
        queue -> msg_array[queue -> tail_idx] = *msg_in; // put the message in the array
        queue -> size++; // increment the size
        if (DEBUG) print_queue(queue); // debug statement
        return queue -> size; // return the current queue size
    }
}

int dequeue (msg_queue* queue, ir_msg* msg_out) { // push message out of queue
    if (queue == NULL) { // handle null pointer
        printk(KERN_INFO "Queue is NULL!\n");
        return -2;
    }
    if (is_empty(queue)) return -1; // if empty do nothing
    else { // if not empty
        *msg_out = queue -> msg_array[queue -> head_idx + 1]; // pass message from array 
        queue -> head_idx = (queue -> head_idx + 1) % queue -> capacity; // head index increment in circular manner
        queue -> size--; // decrement size
        if (DEBUG) { // debug statement
            printk(KERN_INFO "dequeue!\n");
            print_queue(queue);
        }
        return queue -> size; // return current size
    }
}

