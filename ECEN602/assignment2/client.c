//client.c
//Acts as Client
//
//Connects to a remote server
//Initiates a JOIN with server using username supplied on the command line
//Receives ACK/NAK from the server depending upon whether the join request was accepted or rejected by the server
//Reads user input and sends the same to the server
//Receives messages (sent by other clients) from the server
//Receives messages from server which informs the status of other clients in the same chat session
//Monitors its (self) idle time and sends an IDLE message to server when it is idle for more than 10 seconds
//
// Command to start client:
//./client username server_ip server_port
//where username is the username is the name client chooses for himself for the chat session
//server_ip is the IPv4(or IPv6) address of the server
//and server_port is the port number on which the server is listening

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
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include "server.h"

//Defining constants
#define ERROR -1            //Error Value
#define MAXCHAR 512         //Maximum number of characters that should be accepted
#define STDIN 0             //For STDIN
#define VERSION 3           //SBCP Message Protocol Version
#define JOIN 2              //SBCP Message type when JOIN request is being sent
#define SEND 4              //SBCP Message type when chat text is being sent
#define FWD 3               //SBCP Message type when FWD message is being received
#define ACK 7               //SBCP Message type when ACK is being received
#define NAK 5               //SBCP Message type when NAK is being received
#define ONLINE 8            //SBCP Message type when ONLINE message is being received
#define OFFLINE 6           //SBCP Message type when OFFLINE message is being received
#define IDLE 9              //SBCP Message type when IDLE message is being sent/received
#define USERNAME 2          //SBCP Attribute type for the attribute 'Username'
#define MESSAGE 4           //SBCP Attribute type for the attribute 'Message'
#define REASON 1            //SBCP Attribute type for the attribute 'Reason'
#define CLIENT_COUNT 3      //SBCP Attribute type for the attribute 'Client Count'
#define EMPTY 0             //For messages when attributes are absent
#define IDLE_INTERVAL 10     //Idle interval for comparison with self idle time

//SBCP Attribute
struct SBCP_Attribute
{
    short int Type;
    short int Length;
    char Payload[MAXCHAR];
};

//SBCP Message
struct SBCP_Message
{
    uint8_t Vrsn;
    uint8_t Type;
    short int Length;
    struct SBCP_Attribute attributes[2];
};

//err_sys() displays the corresponding error message depending on the err_no value
void err_sys(char *err_msg)
{
    perror(err_msg);
}

//JOIN_CHAT() sends a join request to server with its username
int JOIN_CHAT(int sockfd, char username[])
{
    int sent_bytes;
    struct SBCP_Message msg;
    
    //Packing the structure with the Message and Attribute field details
    msg.Vrsn = VERSION;
    msg.Type = JOIN;
    msg.Length = 2*sizeof(uint8_t) + sizeof(short int) + msg.attributes[0].Length + msg.attributes[1].Length;
    
    msg.attributes[0].Type = USERNAME;
    msg.attributes[0].Length = 2*sizeof(short int)+strlen(username);
    strcpy(msg.attributes[0].Payload,username);
    
    msg.attributes[1].Type = EMPTY;
    msg.attributes[1].Length = 0;
    msg.attributes[1].Payload[0] = '\0';
    
    //Sending the request to the server
    sent_bytes = send(sockfd, (struct SBCP_Message*)&msg, sizeof(msg), 0); //
    if(sent_bytes <= ERROR)
    {
        err_sys("Send Error");  //Checking for errors in sending
    }
    
    return sent_bytes;
}

//SEND_MSG() sends a message to other clients via the server
int SEND_MSG(int sockfd, char* message)
{
    int sent_bytes;
    struct SBCP_Message msg;
    
    //Packing the structure with the Message and Attribute field details
    msg.Vrsn = VERSION;
    msg.Type = SEND;
    msg.Length = 2*sizeof(uint8_t) + sizeof(short int) + msg.attributes[0].Length + msg.attributes[1].Length;
    
    msg.attributes[0].Type = MESSAGE;
    msg.attributes[0].Length = 2*sizeof(short int) + strlen(message);
    strcpy(msg.attributes[0].Payload,message);
    
    msg.attributes[1].Type = EMPTY;
    msg.attributes[1].Length = 0;
    msg.attributes[1].Payload[0] = '\0';
    
    //Sending the message to the server
    sent_bytes = send(sockfd, (struct SBCP_Message*)&msg, sizeof(msg), 0); //
    if(sent_bytes <= ERROR)
    {
        err_sys("Send Error");  //Checking for errors in sending
    }
    
    return sent_bytes;
}

//IDLE_CLIENT() sends a idle message to the server
int IDLE_CLIENT(int sockfd)
{
    int sent_bytes;
    struct SBCP_Message msg;
    
    //Packing the structure with the Message and Attribute field details
    msg.Vrsn = VERSION;
    msg.Type = IDLE;
    msg.Length = 2*sizeof(uint8_t) + sizeof(short int) + msg.attributes[0].Length + msg.attributes[1].Length;
    
    msg.attributes[0].Type = EMPTY;
    msg.attributes[0].Length = 0;
    msg.attributes[0].Payload[0] = '\0';
    
    msg.attributes[1].Type = EMPTY;
    msg.attributes[1].Length = 0;
    msg.attributes[1].Payload[0] = '\0';
    
    //Sending the message to the server
    sent_bytes = send(sockfd, (struct SBCP_Message*)&msg, sizeof(msg), 0); //
    if(sent_bytes <= ERROR)
    {
        err_sys("Send Error");   //Checking for errors in sending
    }
    
    return sent_bytes;
}

int main(int argc, char **argv)
{
    int sockfd; //Socket File Descriptor for client
    struct addrinfo rem_serv,*address;  //Server Address info
    char input[MAXCHAR];
    int sent_bytes, rec_bytes;
    
    struct SBCP_Message packet;
    fd_set readfd;  //fd_set for I/O Multiplexing
    int recv_bytes = 0;
    struct timeval t;   //Wait time for I/O Multiplexing
    time_t last_send, now;  //Variables for checking computing idle time
    int idletime;
    int flag = 0;   //Flag for tracking idle time; Should track idle time when Flag = 1
    
    //Checking for number of input arguments
    if (argc != 4) {
        fprintf(stderr,"One or more of input arguments missing!\n");
        exit(1);
    }
    
    memset(&rem_serv, 0, sizeof(struct addrinfo));  //Initializing the remote server details with zeros
    rem_serv.ai_family = AF_UNSPEC;  //Assigning the IP Family value for the remote server
    rem_serv.ai_socktype = SOCK_STREAM; //Assigning socket type for the remote server
    
    
    if((getaddrinfo(argv[2],argv[3],&rem_serv,&address))<0) //Getting the address
        err_sys("error in getting addr");
    
    if((sockfd = socket(address->ai_family, address->ai_socktype,address->ai_protocol)) <= ERROR) //Creating a socket for client and checking for error
    {
        err_sys("Socket Error");  //Terminates the program in case of an error in socket creation and displays the corresponding message
        exit(-1);
    }
    
    
    if((connect(sockfd, address->ai_addr, address->ai_addrlen)) <= ERROR) //Establishing a connection between server and client and checking for error
    {
        err_sys("Connect Error"); //Terminates the program in case of an error in establishing connection and displays the corresponding message
        exit(-1);
    }
    
    //Initializing variables for I/O multiplexing
    FD_ZERO(&readfd);   //Initializing the readfd fd_set with all zeros
    FD_SET(STDIN, &readfd); //Adding STDIN to the readfd fd_set
    //Initializing the wait time
    t.tv_sec = 10;
    t.tv_usec = 0;
    
    //Client sending a request to the server asking to add it to the chatroom and checking whether the request was sent successfully or not
    if(JOIN_CHAT(sockfd, argv[1]) <= ERROR)
    {
        printf("Client could not send a request to join the chatroom.\n");
        exit(-1);
    }
    else
    {
        fprintf(stdout,"Client sends a request to join the chatroom.\n");
        FD_SET(sockfd, &readfd);    //Adding client socket file descriptor to the readfd fd_set
    }
    time(&last_send);
    flag=1;
    //At this point in time, the client is added to the chatroom. Now, the client receives and sends various messages to the server.
    while(1)
    {
        //Checking for Idle time
        if (flag == 1)
        {
            //Getting the current system time
            time(&now);
            
            //Computing the idle time and comparing it with the maximum idle interval allowed
            if((idletime = (int)difftime(now,last_send)) >= IDLE_INTERVAL)
            {
                //Sending an IDLE message to server when the client is idle for more than allowable idle time and checks whether the IDLE message was sent successfully or not
                if(IDLE_CLIENT(sockfd) <= ERROR)
                {
                    printf("Error sending IDLE Packet\n");
                }
                flag = 0;   //Resetting the flag to stop tracking the idle time
            }
        }
        
        //Implementing I/O multiplexing - Client is checking the socket file descriptor and standard input for any activity. If it doesn't sense any activity for a period of wait time referred by 't', it should return.
        select(sockfd+1, &readfd, NULL, NULL, &t);
        
        //Checking for activity on Standard input
        if(FD_ISSET(STDIN, &readfd))
        {
            if(fgets(input, (MAXCHAR-1), stdin)!=NULL)  //Takes input from user and exchanges the data only when the fgets() gives what user has entered
            {
                if(feof(stdin)) //Checks for EOF (Ctrl+D) and stops the data transfer (when EOF is encountered after a message)
                    break;
                
                //Sends the message typed by user on client side to the server so that it is forwarded to all the other users and checks whether the message has been sent successfully
                if((SEND_MSG(sockfd, input)) <= ERROR)
                {
                    printf("Send Error\n");
                }
                    time(&last_send);   //Resetting the time when a message was last sent by the client
                    flag = 1;   //Setting the flag to 1 for enabling tracking of idle time
                
                if(feof(stdin)) //Checks for EOF (Ctrl+D) and stops the data transfer (when EOF is encountered initially)
                    break;
            }
        }
        
        //Checking for activity on socket file descriptor
        else if(FD_ISSET(sockfd, &readfd))
        {
            if((recv_bytes = recv(sockfd, (struct SBCP_Message*)&packet, sizeof(struct SBCP_Message), 0)) <= 0)
            {
                /*if (rec_bytes == 0)
                {
                    err_sys("Connection is closed by the server \n");
                }   
                else
                {
                    err_sys("Receive Error");  //Terminates the program in case of an error in receiving the message and displays the corresponding error message
                    break;
                    
                }*/
                printf("Connection closed by the server \n");
                break;
            }
            
            else if (recv_bytes >= packet.Length)   //Checking if all intended bytes have been received
            {
                if(packet.Vrsn == VERSION)  //Checking for version in the received packet
                {
                    //When a FWD message is received
                    if((packet.Type == FWD) && (packet.attributes[0].Type == USERNAME) && (packet.attributes[1].Type == MESSAGE) )
                    {
                    	if (strcmp(packet.attributes[1].Payload, "N/A")) 
                        	printf("%s: %s\n", packet.attributes[0].Payload, packet.attributes[1].Payload);
                    }
                    
                    //When a ACK is received
                    else if((packet.Type == ACK) && (packet.attributes[0].Type == CLIENT_COUNT) && (packet.attributes[1].Type == USERNAME))
                    {
                    	if (strcmp(packet.attributes[1].Payload, "N/A")) 
                        	printf("Client has joined the chatroom. There are %s members in the chatroom - %s", packet.attributes[0].Payload, packet.attributes[1].Payload);
                    }
                    
                    //When a NAK is received
                    else if((packet.Type == NAK) && (packet.attributes[0].Type == REASON))
                    {
                        std::cout << packet.attributes[0].Payload <<std::endl;
                        break;

                    }
                    
                    //When a ONLINE message is received
                    else if((packet.Type == ONLINE) && (packet.attributes[0].Type == USERNAME))
                    {
                        printf("***%s has joined the chatroom.***\n", packet.attributes[0].Payload);
                    }
                    
                    //When a OFFLINE message is received
                    else if((packet.Type == OFFLINE) && (packet.attributes[0].Type == USERNAME))
                    {
                        printf("***%s has left the chatroom.***\n", packet.attributes[0].Payload);
                    }
                    
                    //When an IDLE message is received
                    else if((packet.Type == IDLE) && (packet.attributes[0].Type == USERNAME))
                    {
                        printf("%s is idle.", packet.attributes[0].Payload);
                    }
                }
                
            }
        }
        
        //Adding standard input and socket file descriptor to the fd_set readfd
        FD_SET(STDIN,&readfd);
        FD_SET(sockfd,&readfd);
        
    }
    
    close(sockfd);  //Closes the Client Socket
    fprintf(stdout,"Closing client socket");    //Displays message to the user regarding the closure of client socket
    return 0;
}
