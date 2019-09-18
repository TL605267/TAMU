#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    unsigned int read_i, read_j, result;
    int fd;     // File descriptor 
    int i, j;   // Loop variables
    // allocate read buffer
    char* rd_buf = (char*) malloc(3 * sizeof(int));
	// allocate write buffer
    int* wr_buf = (int*) malloc(2 * sizeof(int));
    unsigned int* int_rd_buf; // int pointer for read buffer
    char input = 0; // input from user
    
    // Open device file for reading and writing 
    // Use 'open' to open '/dev/multiplier'
    fd = open("/dev/multiplier", O_RDWR);
    // Handle error opening file 
    if(fd == -1) {
        printf("Failed to open device file!\n");
        return -1;
    }
    
    for(i = 0; i <= 16; i++) {
        for(j = 0; j <= 16; j++) {
            // Write value to registers using char dev 
            // Use write to write i and j to peripheral 
            wr_buf[0] = i; // assign to write buffer
            wr_buf[1] = j; // assign to write buffer
            write(fd, (char*)wr_buf, 2 * sizeof(int)); // write to device
            // Read i, j, and result using char dev
            // Use read to read from peripheral 
            read(fd, rd_buf, 3 * sizeof(int)); // read from device
            int_rd_buf = (unsigned int*) rd_buf; // cast to unsigned int
            read_i = int_rd_buf[0]; // assign to read buffer
            read_j = int_rd_buf[1]; // assign to read buffer
            result = int_rd_buf[2]; // assign to read buffer
            // print unsigned ints to screen 
            printf("%u * %u = %u\n\r", read_i, read_j, result);
                
            // Validate result 
            if(result == (i*j))
                printf("Result Correct!");
            else
                printf("Result Incorrect!");
                
            // Read from terminal 
            input = getchar();
            // Continue unless user entered 'q' 
            if(input == 'q') {
                close(fd);
                return 0;
            }
        }
    }
    close(fd); // close device
    free(wr_buf); // free write buffer
    free(rd_buf); // free read buffer
    return 0;
}
