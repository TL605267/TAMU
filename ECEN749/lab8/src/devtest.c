#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// arm-xilinx-linux-gnueabi-gcc -o devtest devtest.c 
#define MAX_STRING_SIZE 100
int main() {
    unsigned short read_0, read_1;
    int fd;     // File descriptor 
    unsigned char* rd_buf = (unsigned char*) malloc(MAX_STRING_SIZE*2*sizeof(char));
    char input[3] = "0";
    int data;
    int i, buf_read_len;
    unsigned short* msg;
    int input_num = 0;
    // Open device file for reading and writing 
    // Use 'open' to open '/dev/multiplier'
    fd = open("/dev/ir_demod", O_RDWR);
    // Handle error opening file 
    if(fd == -1) {
        printf("Failed to open device file!\n");
        return -1;
    }
    while (input != "q") { // quit when typing in "q"
        printf("message Reading...\n");
        input_num = atoi(input); // change input type from string to integer, if possible
        if (input_num) { // if input varible is valid
            buf_read_len = read(fd, rd_buf, input_num); // read input_num bits from ir_demod
            if (buf_read_len) { // if read succeed
                msg=(unsigned short*)rd_buf; // convert read buffer to two byte type
                for (i = 0; i < buf_read_len; i++) { // iterate through the buffer
                    data=msg[i]&0xfff; // read out only the lower 12 bits
                    printf("message %d = 0x%x\n", (i+1), (unsigned int)data); // print out the data
                }
            }
        }
        // Read from terminal 
        printf("Enter number of message you want to read:"); // print statement for user interface. 
        fgets(input, MAX_STRING_SIZE, stdin); // get user input for the number of message user want to read
        // Continue unless user entered 'q' 
    }
    close(fd);
    free(rd_buf);
    return 0;
}
