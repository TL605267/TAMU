// ./serv port

//Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
//#include <unistd.h>
#include <netdb.h>

//Defining constants
#define ERROR -1    //Error Value
#define MAXCHAR 50  //Maximum number of characters that should be accepted
#define MAXCLIENT 3 //Maximum number of clients that the server could handle

//err_sys() displays the corresponding error message depending on the err_no value
void err_sys(char *err_msg)
{
    perror(err_msg);
}

////////////Start               Need to

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
    int sockfd;
    struct addrinfo server, *servinfo, *res;  //
    int val, recv_bytes, length, sent_bytes, count, errnum, read_flag = 0, write_flag = 0, stop_flag = 0;
    struct sockaddr_storage client_addr;
    char packet[MAXCHAR], ErrorCode[10], ErrMsg[50], ACK[10], BlockNo[10], Mode[10], Filename[10], pack[20],ch, nextch, data[MAXCHAR], errmsg[100];
    FILE *fp;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    
    port = argv[2];
    
    memset(&server, 0, sizeof(server));
    server.ai_family = AF_UNSPEC;
    server.ai_socktype = SOCK_DGRAM;
    server.ai_protocol = IPPROTO_UDP;
    server.ai_flags = AI_PASSIVE;
    
    if ((val = getaddrinfo(NULL, port, &server, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(val));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
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
    printf("waiting to recvfrom...\n");
    addr_len = sizeof(client_addr);
    
    ///////////// while & fork
    if ((recv_bytes = recvfrom(sockfd, packet, MAXCHAR-1 , 0, (struct sockaddr *)&client_addr, &addr_len)) <= ERROR)
    {
        perror("Error in receiving:");
        exit(1);
    }
    else
    {
        length = sizeof(packet);
        if(strncmp(packet,"01",2) == 0)         //RRQ
        {
            read_flag = 1;
            write_flag = 0;
            fscanf(packet,"%s0%s0", pack, Mode);
            strncpy(Filename, pack1+3);
            fp = fopen(Filename, "r");
            
            if(fp == NULL)
            {
                strerror_r(errnum, errmsg, sizeof(errmsg));
                if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending Error Packet:");
                    exit(1);
                }
                else
                    break;      //////To close the sockfd
            }
            
            nextch = -1;
            for(count = 0; count < 512; count++)
            {
                if(nextch >= 0)
                {
                    data[count+1] = nextch;
                    nextch = -1;
                    continue;
                }
                ch = getc(fp);
                
                if(ch == EOF)
                {
                    if(ferror(fp))
                    {
                        strcpy(errmsg, "0501File not found.0");
                        if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                        {
                            perror("Error sending Error Packet:");
                            exit(1);
                        }
                        else
                            break;      //////To close the sockfd
                    }
                    else if(feof(fp))
                    {
                        if(count%512 == 0)
                        {
                            /////////Send dummy packet
                        }
                        else
                        {
                        stop_flag = 1;
                        break;
                        }
                    }
                    //                    return(count);
                }
                else if(ch == '\n')
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
                data[count+1] = c;
            }
            
            if ((sent_bytes = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
            {
                perror("Error sending Data Packet:");
                exit(1);
            }
            else
                printf("Packet 1 sent.\n");
        }
        else if(strncmp(packet,"02",2) == 0)    //WRQ
        {
            read_flag = 0;
            write_flag = 1;
            
            fscanf(packet,"%s0%s0", pack, Mode);
            strncpy(Filename, pack1+3);
            fp = fopen(Filename, "wx");
            if(fp == NULL)
            {
                strerror_r(errnum, errmsg, sizeof(errmsg));
                if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending Error Packet:");
                    exit(1);
                }
                else
                    break;      //////To close the sockfd
            }
            else
            {
                strcpy(ACK,"0400");
                if ((sent_bytes = sendto(sockfd, ACK, strlen(ACK), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending ACK:");
                    exit(1);
                }
            }
        }
        else if(strncmp(packet,"03",2) == 0)    //DATA
        {
            if((read_flag == 1) || (write_flag == 0))
            {
                strcpy(errmsg, "0502Access violation.0");
                if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending Error Packet:");
                    exit(1);
                }
                else
                    break;      //////To close the sockfd
            }
            else
            {
                strncpy(BlockNo, packet+3, 2);
                strcpy(ACK,"04");
                strcat(ACK, BlockNo);
                if ((sent_bytes = sendto(sockfd, ACK, strlen(ACK), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending ACK:");
                    exit(1);
                }
                
                
                BlockNo = atoi(BlockNo);
                for(count = (BlockNo-1)*512; count < BlockNo*512; count++)
                {
                    ch = data[count];
                    putc(ch, fp);
                    
                    if(ch == EOF)
                    {
                        if(ferror(fp))
                        {
                            strcpy(errmsg, "0506File already exists.0");
                            if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                            {
                                perror("Error sending Error Packet:");
                                exit(1);
                            }
                            else
                                break;      //////To close the sockfd
                        }
                        else if(feof(fp))
                        {
                            if(count%512 == 0)
                            {
                                ///////Check for dummy packet
                            }
                            else
                            {
                                stop_flag = 1;
                                break;
                            }
                        }
                        //                    return(count);
                    }

                
                
                
            }
        }
        else if(strncmp(packet,"04",2) == 0)    //ACK
        {
            if((read_flag == 0) || (write_flag == 1))
            {
                strcpy(errmsg, "0502Access violation.0");
                if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending Error Packet:");
                    exit(1);
                }
                else
                    break;      /////To close sockfd
            }
            else
            {
                strncpy(BlockNo, packet+3, 2);
                BlockNo = atoi(BlockNo);
                
                if(stop_flag = 0)
                {
                    nextch = -1;
                    for(count = BlockNo*512; count < (BlockNo+1)*512; count++)
                    {
                        if(nextch >= 0)
                        {
                            data[count+1] = nextch;
                            nextch = -1;
                            continue;
                        }
                        ch = getc(fp);
                        
                        if(ch == EOF)
                        {
                            if(ferror(fp))
                            {
                                strcpy(errmsg, "0501File not found.0");
                                if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                                {
                                    perror("Error sending Error Packet:");
                                    exit(1);
                                }
                                else
                                    break;      //////To close the sockfd
                            }
                            else if(feof(fp))
                            {
                                if(count%512 == 0)
                                {
                                    ////////Send dummy packets
                                }
                                else
                                {
                                stop_flag = 1;
                                break;
                                }
                            }
                            //                        return(count);
                        }
                        else if(ch == '\n')
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
                        data[count+1] = c;
                    }
                }
                if ((sent_bytes = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending ACK:");
                    exit(1);
                }
                else
                    printf("Packet %d sent.\n", BlockNo+1);
            }
        }
        else if(strncmp(packet,"05",2) == 0)    //ERROR
        {
            if((read_flag == 0) && (write_flag == 0))
            {
                strcpy(errmsg, "0502Access violation.0");
                if ((sent_bytes = sendto(sockfd, errmsg, strlen(errmsg), 0, (struct sockaddr *)&client_addr, addr_len)) <= ERROR)
                {
                    perror("Error sending Error Packet:");
                    exit(1);
                }
                else
                    break;      /////To close sockfd
            }
            else
            {
                strncpy(ErrorCode, packet+3, 2);
                strncpy(ErrMsg, packet+5, length-5);
                printf("Error %s: %s\n", ErrorCode, ErrMsg);
                break;          ///////Terminate or not
            }
        }
    }
    if(stop_flag == 1)
    {
        fclose(fp);
        break;                  //////To close the sockfd
    }
    
    printf("listener: got packet from %s\n",
           inet_ntop(client_addr.ss_family,
                     get_in_addr((struct sockaddr *)&client_addr),
                     s, sizeof s));
    printf("listener: packet is %d bytes long\n", recv_bytes);
    buf[recv_bytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);
    close(sockfd);
    return 0;
}
