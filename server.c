//tftps.c
// Trivial File Transfer Protocol (TFTP) Server
//
// Transfers files between two systems in both directions using the User Datagram Protocol (UDP)
// Can handle multiple TFTP clients
//
// Command to start server:
// ./tftp Server_IP_Addr Port
// where IP_Addr is the IPv4 address of the server in dot decimal notation
// and Port is the port number on which the server is listening (should be other than the famous port 69 for this assignment)


//Standard Libraries
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <signal.h>

//Defining constants
#define ERROR -1        //Error Value
#define MAXCHAR 1000    //Maximum number of characters that should be accepted
#define MAXCLIENT 3     //Maximum number of clients that the server could handle
#define HEADER 4        //Size of Header
#define TIMEOUT 1       //Timeout value (1 second)
#define NOBYTES 512     //Number of bytes in a packet

//err_sys() displays the corresponding error message depending on the err_no value
void err_sys(char *err_msg)
{
    perror(err_msg);
}

//timeout() sets the timer upon transmitting a data packet for servicing timeout
int timeout(int fd, int sec)
{
    fd_set rset;
    struct timeval tv;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    
    return select(fd+1, &rset, NULL, NULL, &tv);
}

//sigchld_handler() reaps the zombie processes
void sigchld_handler(int s)
{
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

//*get_in_addr() helps support IPv6 and IPv4 addresses
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//main()
int main(int argc, char **argv)
{
    //Declaring and intializing the variables used in the program
    int sockfd, childsockfd;
    struct addrinfo server, *servinfo, *res, child, *childinfo;  //
    int val, childval, recv_bytes, length, sent_bytes, count, errnum, read_flag = 0, write_flag = 0, stop_flag = 0, final = -1, mode_offset,yes=1, size_sp, Intermediate = 0; mode_flag = -1, time_count = 0;
    unsigned short sent_num;
	struct sockaddr_storage client_addr;
    struct sockaddr_in addr;
    char packet[MAXCHAR], ErrorCode[10], ErrMsg[50], ACK[10], BlockNo[10], Mode[10], Filename[10], pack[20],ch, nextch = -1, data[MAXCHAR], errmsg[100], datapack[517], send_pack[517], childport[10], msg[50];
    FILE *fp;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    struct sigaction sa;
    
    //Server attempts to connect to the given port using datagram socket
    memset(&server, 0, sizeof(server));
    server.ai_family = AF_UNSPEC;       //Supports both the IPv4 and IPv6 family
    server.ai_socktype = SOCK_DGRAM;    //Datagram socket
    server.ai_protocol = IPPROTO_UDP;   //Follows UDP protocol
    server.ai_flags = AI_PASSIVE;       //Returned socket address structure is intended for used for binding
    
    //Fetches a list of IP addreses and port numbers
    if ((val = getaddrinfo(argv[1], argv[2], &server, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(val));
        return 1;
    }
    
    //Looping through all the results and bind to the first
    for(res = servinfo; res != NULL; res = res->ai_next)
    {
        if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) <= ERROR)
        {
            perror("Socket Error:");
            continue;
        }
        if (bind(sockfd, res->ai_addr, res->ai_addrlen) <= ERROR)
        {
            close(sockfd);
            perror("Bind Error:");
            continue;
        }
        break;
    }
    if (res == NULL)
    {
        fprintf(stderr, "Failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    
    //For reaping all the zombie processes
    sa.sa_handler = sigchld_handler; // reaps all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    
    printf("waiting to recvfrom...\n");
    addr_len = sizeof(client_addr);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    while(1)
    {
        //Listening scoket receiving the packet over the network from the client
        if ((recv_bytes = recvfrom(sockfd, packet, sizeof(packet) , 0, (struct sockaddr *)&client_addr, &addr_len)) == ERROR)
        {
            perror("Error in receiving:");
            exit(1);
        }
        else
        {
            //fprintf(stdout,"Debug-packet opcode is:::: %d \n",packet+1);
            if (!fork())
            {
                // Inside the child process
                
                memset(&addr, 0, sizeof addr);
                inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(0);
                
                //Creating socket in child process
                if ((childsockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
                {
                    perror("child_socket error");
                    exit(1);
                }
                if (setsockopt(childsockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
                {
                    perror("child_setsockopt error");
                    exit(1);
                }
                if (bind(childsockfd, (struct sockaddr *)&addr, sizeof addr) == -1)
                {
                    close(childsockfd);
                    perror("bind error");
                    exit(1);
                }
                
                
                close(sockfd); //Listener leaves/doesn't involve in the transfer process
                
                length = sizeof(packet);
                //fprintf(stdout,"Packet received on main socket is %s",packet);
                
                //////////////////////////////////////////////////////////////////////////////////////
                
                if(packet[1]==1)    //RRQ
                {
                    //fprintf(stdout,"Debug - Entered RRQ");
                    read_flag = 1;  //Client requested for reading
                    write_flag = 0;
                    
                    memset(send_pack, 0, sizeof(send_pack));
                    
                    //Parsing the filename and mode requested by the client
                    strcpy(Filename,packet+2);
                    mode_offset = 2+strlen(Filename)+1;
                    strcpy(Mode,packet+mode_offset);
                    
                    if((strcmp(Mode, "octet") == 0) || (strcmp(Mode, "OCTET") == 0))
                    {
                        mode_flag = 0;
                    }
                    else if((strcmp(Mode, "netascii") == 0) || (strcmp(Mode, "NETASCII") == 0))
                    {
                        mode_flag = 1;
                    }
                    else
                    {
                        //If the mode is neither octet nor netascii, the server sends an error packet
                        sprintf(errmsg, "%c%c%c%c", 0x00, 0x05, 0x00, 0x04);    //Building the error packet
                        strcpy(msg, "Illegal TFTP operation.");
                        memcpy(errmsg+4, msg, strlen(msg));
                        memcpy(send_pack,errmsg, strlen(msg)+HEADER);
                        size_sp = strlen(msg)+HEADER;
                        
                        //Transmitting the packet over the network to the client
                        if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                        {
                            perror("Error sending Error Packet:");
                            exit(1);
                        }
                        else
                        {
                            stop_flag = 1;
                            break;          //Stop the transfer if an error is encountered
                        }
                    }
                    //Opening the file requested by client in read mode
                    fp = fopen(Filename, "r");
                    
                    //If the file does not exist, server sends an error packet
                    if(fp == NULL)
                    {
                        //Building the error packet
                        sprintf(errmsg, "%c%c%c%c", 0x00, 0x05, 0x00, 0x01);
                        strcpy(msg, "File not found");
                        memcpy(errmsg+4, msg, strlen(msg));
                        
                        memcpy(send_pack, errmsg, strlen(msg)+HEADER);
                        size_sp = strlen(msg)+HEADER;
                        
                        //Transmitting the packet over the network to the client
                        if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                        {
                            perror("Error sending Error Packet:");
                            exit(1);
                        }
                        else
                        {
                            stop_flag = 1;  //Stop the transfer if an error is encountered
                            break;
                        }
                    }
                    
                    //Reads the file according in the mode requested by the client
                    for(count = 0; count < NOBYTES; count++)
                    {
                        if(nextch >= 0)
                        {
                            data[count] = nextch;
                            nextch = -1;
                            continue;
                        }
                        
                        ch = getc(fp);  //Fetching the character from the file
                        
                        if(ch == EOF)
                        {
                            if(ferror(fp))
                                printf("read error from fgetc");
                            stop_flag = 1;  //Stop the transfer once the file has been read completely
                            break;
                        }
                        
                        //Inserting a Carriage Return (CR) character before every Line Fee (LF) character for the case when mode is NETASCII
                        if(mode_flag == 1)
                        {
                            if(ch == '\n')
                            {
                                ch = '\r';
                                nextch = '\n';
                            }
                            else if(ch == '\r')
                            {
                                nextch = '\0';
                            }
                            else
                            {
                                nextch = -1;
                            }
                        }
                        
                        data[count] = ch;   //Storing the read character in the data buffer
                        if(stop_flag == 1)
                            break;
                    }
                    
                    sprintf(datapack,"%c%c%c%c",0x00,0x03,0x00,0x01);   //Building the header of the data packet to be sent
                    memcpy(datapack+4, data, count);    //Completing the data packet with the data
                    //fprintf(stdout,"Debug: data = %s\n",data);
                    memcpy(send_pack,datapack,count+HEADER);
                    //printf("\n RRQ Debug: datapack = %s \n RRQ Debug: Send packet = %s \n",datapack,send_pack);
                    //fprintf(stdout,"Debug: data in datapack = %s\n",datapack[4]);
                    size_sp = count+HEADER;
                    
                    //Transmitting the packet over the network to the client
                    if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                    {
                        perror("Error sending Data Packet:");
                        exit(1);
                    }
                    else
					{
                    	block_number=1;
                        //printf("Debug: Packet 0 sent.\n");
                    }
                }
                
                //////////////////////////////////////////////////////////////////////////////////////
                
                else if(packet[1] == 2)    //WRQ
                {
                    
                    fprintf(stdout, "Debug - Entered WRQ");
                    read_flag = 0;
                    write_flag = 1;     //Client requested for reading
                    
                    //Parsing the filename and mode requested by the client
                    strcpy(Filename,packet+2);
                    mode_offset = 2+strlen(Filename)+1;
                    strcpy(Mode,packet+mode_offset);
                    
                    memset(send_pack, 0, sizeof(send_pack));
                    
                    //Opening the file requested by client in write mode
                    fp = fopen(Filename, "wx");
                    
                    //If the file already exists, server sends an error packet
                    if(fp == NULL)
                    {
                        //Building the error packet
                        sprintf(errmsg, "%c%c%c%c", 0x00, 0x05, 0x00, 0x06);
                        strcpy(msg, "File already exists.");
                        memcpy(errmsg+4, msg, strlen(msg));
                        memcpy(send_pack,errmsg, strlen(msg)+HEADER);
                        size_sp = strlen(msg)+HEADER;
                        
                        //Transmitting the packet over the network to the client
                        if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                        {
                            perror("Error sending Error Packet:");
                            exit(1);
                        }
                        else
                        {
                            stop_flag = 1;      //Stop the transfer if an error is encountered
                            break;
                        }
                    }
                    //If the file does not exist, server sends an an acknowledgement accepting the client request
                    else
                    {
                        //Building the ACK packet
                        sprintf(ACK, "%c%c%c%c", 0x00, 0x04, 0x00, 0x00);
                        memcpy(send_pack, ACK, HEADER);
                        size_sp = HEADER;
                        
                        //Transmitting the packet over the network to the client
                        if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                        {
                            perror("Error sending ACK:");
                            exit(1);
                        }
                        else
                        {
                            printf("Debug: ACK sent.\n");
                            block_number = 1;
                        }
                        
                    }
                }
                
                //////////////////////////////////////////////////////////////////////////////////////
                
                while(1)
                {
                    if(timeout(childsockfd,1) == 0)
                    {
                        //                    if((read_flag == 1) && (write_flag == 0))
                        //                    {
                        //Retransmitting the packet over the network to the client
                        if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                        {
                            perror("Error sending Error Packet:");
                            exit(1);
                        }
                        else
                        {
                            printf("Socket timed out\n");
                            time_count++;
                            if(time_count >= 10)
                            {
                                stop_flag = 1;  //Stop the transfer if number of timeouts exceed 10
                                printf("Exceeded the maximum number of timeouts allowed. Ending the connection.\n");
                                break;
                            }
                        }
//                    }
                    }
                    else
                    {
                        //Child scoket receiving the packet over the network from the client
                        if ((recv_bytes = recvfrom(childsockfd, packet, sizeof(packet) , 0, (struct sockaddr *)&client_addr, &addr_len)) <= ERROR)
                        {
                            perror("Error in receiving:");
                            exit(1);
                        }
                        time_count = 0;     //Resetting the timeout counter
                        length = sizeof(packet);
                        
                        //////////////////////////////////////////////////////////////////////////////
                        
                        if(packet[1] == 3)    //DATA
                        {
                            memset(send_pack, 0, sizeof(send_pack));
                            memset(BlockNo, 0, sizeof(BlockNo));
                            memset(data, 0, sizeof(data));
                            memset(ACK, 0, sizeof(ACK));
                            
                            //If the clients sends an ACK packet when it had originally requested for writing a file, server sends an error
                            if((read_flag == 1) || (write_flag == 0))
                            {
                                //Building the error packet
                                sprintf(errmsg, "%c%c%c%c", 0x00, 0x05, 0x00, 0x02);
                                strcpy(msg, "Access violation.");
                                memcpy(errmsg+4, msg, strlen(msg));
                                memcpy(send_pack,errmsg, strlen(msg)+HEADER);
                                size_sp = strlen(msg)+HEADER;
                                
                                //Transmitting the packet over the network to the client
                                if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                                {
                                    perror("Error sending Error Packet:");
                                    exit(1);
                                }
                                else
                                {
                                    stop_flag = 1;  //Stop the transfer if an error is encountered
                                    break;
                                }
                            }
                            else
                            {
                                //Parsing the received data packet
                                memcpy(BlockNo, packet+2, 2);
                                memcpy(data, packet+4, length-4);
                                
                                //Writing the data into a file
                                for(count = 0; count < NOBYTES; count++)
                                {
                                    if(nextch >= 0)
                                    {
                                        putc(nextch, fp);
                                        nextch = -1;
                                        continue;
                                    }
                                    
                                    ch = data[count];   //Getting the data from the packet
                                    
                                    if(ch == EOF)
                                    {
                                        stop_flag = 1;     //Stop the transfer if file has been written completely
                                        break;      //Does this break need to be placed here or after  the EOF has been written to the file?
                                    }
                                    //Inserting a Carriage Return (CR) character before every Line Fee (LF) character for the case when mode is NETASCII
                                    if(mode_flag == 1)
                                    {
                                        if(ch == '\n')
                                        {
                                            ch = '\r';
                                            nextch = '\n';
                                        }
                                        else if(ch == '\r')
                                        {
                                            nextch = '\0';
                                        }
                                        else
                                        {
                                            nextch = -1;
                                        }
                                    }

                                    //Writing the data character into the file
                                    putc(ch, fp);
                                    
//                                    if(stop_flag == 1)
//                                        break;
                                }
                                
                                //Building an ACK packet for the received data packet
                                sprintf(ACK, "%c%c", 0x00, 0x04);
                                Intermediate = (block_number+1)%65536;
                                sent_num = Intermediate;
                                datapack[2]=(sent_num & 0xFF00) >> 8;
                                datapack[3]=(sent_num & 0x00FF);
                                memcpy(send_pack, ACK, HEADER);
                                size_sp = HEADER;
                                
                                //Transmitting the packet over the network to the client
                                if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                                {
                                    perror("Error sending ACK:");
                                    exit(1);
                                }
                                else
                                    printf("Debug: ACK sent.\n");
                                
                                if(block_number == final)
                                {
                                    stop_flag = 1;  //Stop the transfer if writing the file is completed
                                    break;
                                }
                            }
                        }
                        //////////////////////////////////////////////////////////////////////////////
                        else if(packet[1] == 04)    //ACK
                        {
                        	//printf("\n Debug: Naa shaardam ACK lo unna \n");
                            memset(send_pack, 0, sizeof(send_pack));
                            memset(BlockNo, 0, sizeof(BlockNo));
                            memset(data, 0, sizeof(data));
                            memset(ACK, 0, sizeof(ACK));
                            
                            memset(datapack, 0, sizeof(datapack));
                            
                             //If the clients sends a data packet when it had originally requested for reading a file, server sends an error
                            if((read_flag == 0) || (write_flag == 1))
                            {
                                //Building the error packet
                                sprintf(errmsg, "%c%c%c%c", 0x00, 0x05, 0x00, 0x02);
                                strcpy(msg, "Access violation.");
                                memcpy(errmsg+4, msg, strlen(msg));
                                memcpy(send_pack,errmsg, strlen(msg)+HEADER);
                                size_sp = strlen(msg)+HEADER;
                                
                                //Transmitting the packet over the network to the client
                                if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                                {
                                    perror("Error sending Error Packet:");
                                    exit(1);
                                }
                                else
                                {
                                    stop_flag = 1;  //Stop the transfer if an error is encountered
                                    break;
                                }
                            }
                            else
                            {
                                //Parsing the received ACK packet
                            	//printf("\n Debug: Naa shaardam ACK execute anta \n");
                                memcpy(BlockNo, packet+2, 2);
                                //printf("\n Debug: the ACK sent by client is %d \n",block_number);
                                
                               //nextch = -1;     //Needs to be removed
                                //printf("The Block Number is %d  Debug: Stop Flag = %d",block_number,stop_flag);
//                                printf("Debug: I am there");
                                if(stop_flag == 0)
                                {
//                                	printf("Debug: Stop flag : %d",stop_flag);
//                               printf("Debug: I am here1\n");
//                               printf("Debug: I am here2\n");
//                               printf("Debug: I am here3\n");
                                    //Reading the data from the file
                                    for(count = 0; count < NOBYTES; count++)
                                    {
//                                    	printf("Debug: Count = %d\n",count);
                                    	
                                        /*if(count == 2474){
                                    		printf("I am hereeeeeeeeee!!!\n");
                                    		continue;
                                    	}*/
                                        
                                        if(nextch >= 0)
                                        {
                                            data[count] = nextch;
                                            nextch = -1;
//                                            printf("Debug: if(nextch >= 0)!!!\n");
                                            continue;
                                        }
                                        
                                        ch = getc(fp);      //Fetching the character from the file
//                                        printf("Debug: ((((((%c))))))",ch);
                                        
                                        /*if((ch == EOF))
                                        {
                                                if(count%512 == 0)
                                                {
                                                    nextch = '\0';    //dummy packet
                                                    //final = block_number+2;
                                                }
                                                else
                                                {
                                                    //final = block_number+1;
                                                }
                                            //                        return(count);
                                        }*/
                                        
					                    if(ch == EOF)
										 {
										 	//printf("Debug: Finalllllllll");
										 	//printf("Debug: Finalllllllll");
										 	//printf("Debug: Finalllllllll");
										 	//printf("Debug: Finalllllllll");
											 if(ferror(fp))
                                                 printf("read error from fgetc");
											 stop_flag = 1;     //Stop the transfer if file is read has been completely
											 //printf("Debug: I am at break now\n");
											 break;
										 }
                                        
                                        //Inserting a Carriage Return (CR) character before every Line Fee (LF) character for the case when mode is NETASCII
                                        if(mode_flag == 1)
                                        {
                                            if(ch == '\n')
                                            {
                                                ch = '\r';
                                                nextch = '\n';
                                            }
                                            else if(ch == '\r')
                                            {
                                                nextch = '\0';
                                            }
                                            else
                                            {
                                                nextch = -1;
                                            }
                                        }
                                        
                                        data[count] = ch;   //Storing the character in a data buffer
                                        // printf("Debug: Outside\n");
                                        
                                        //if((final == block_number+2) || (final == block_number+1))
                                           // break;
                                    }
                                    
                                    //Building the next data packet to be sent
                                    sprintf(datapack,"%c%c",0x00,0x03);
                                    Intermediate = (block_number+1)%65536;
                                    sent_num = Intermediate;
							 		datapack[2]=(sent_num & 0xFF00) >> 8;
							 		datapack[3]=(sent_num & 0x00FF);
                                   // sprintf(datapack+2, "%c%d", 0x00,Intermediate);
                                    memcpy(datapack+4, data, count);
                                    memcpy(send_pack,datapack,count+HEADER);
                                    size_sp = count+HEADER;
									//printf("\n Debug: datapack = %s \n Debug: Send packet = %s \n",datapack,send_pack);
                                    
                                    //Transmitting the packet over the network to the client
                                    if ((sent_bytes = sendto(childsockfd, send_pack, size_sp, 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                                    {
                                        perror("Error sending Data Packet:");
                                        exit(1);
                                    }
                                    else
                                    {
                                        //printf("Debug: Dupli: Packet %d sent via ACK.\n", block_number+1);
                                    block_number++;
                                    }
                                }
                                else{
                                //printf("Debug: I am breaking dude!\n");
                                //getc(stdout); //Debug
                                    break;  //Stop the transfer
                                }
                            }
                        }
                        
                        //////////////////////////////////////////////////////////////////////////////
                        
                        else if(packet[1] == 5)    //ERROR
                        {
                            memset(ErrorCode, 0, sizeof(ErrorCode));
                            memset(ErrMsg, 0, sizeof(ErrMsg));
                            
                            //Parsing the error packet received from the client
                            memcpy(ErrorCode, packet+2, 2);
                            memcpy(ErrMsg, packet+4, length-4);
                            printf("Error %s: %s\n", ErrorCode, ErrMsg);
                            
                            stop_flag = 1;      //Stop the transfer if an error is encountered
                            break;
                        }
                    }
                }
                
                //////////////////////////////////////////////////////////////////////////////////////
                
                //Closing the connection and cleaning up the resources when the transfer stops
                if(stop_flag == 1)
                {
                    fclose(fp);         //Closes the file
                    close(childsockfd); //Closing the child server
                    exit(0);
                }
            }
        }
    }
    close(sockfd);      //Need to remove
    return 0;
}
