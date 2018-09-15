//echo.c
//Acts as Client
//
//Connects to a remote server
//Reads user input and sends the same to the remote server
//Displays the data lines echoed by the remote server
//
// Command to start client:
//echo IP_Addr Port
//where IP_Addr is the IPv4 address of the server in dot decimal notation
//and Port is the port number on which the server is listening

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

//Defining constants
#define ERROR -1    //Error Value
#define MAXCHAR 50  //Maximum number of characters that should be accepted

//err_sys() displays the corresponding error message depending on the err_no value
void err_sys(char *err_msg)
{
    perror(err_msg);
}

//writen() writes the message (buffer) into the socket file descriptor and returns the number of bytes written or a -1 on error. It also checks for EINTR and calls the function again
int writen(int sockfd, char *buffer, int nbytes)
{
    int no_bytes = 0, nbytes_written = 0;   /*no_bytes corresponds to total number of bytes written; nbytes_written corresponds to number of bytes written during a particular write call.*/
    char *buf;
    buf= buffer;
    
    while(no_bytes < nbytes)
    {
        if((nbytes_written = write(sockfd, buf, (nbytes - no_bytes))) <=0)  //Writing the message and checking for error
        {
            if(errno == EINTR)
            {
                continue;   //Recalling the function when an EINTR occurs
            }
            else
            {
				return(-1); //Returns an error value in case of an error
            }
        }
        no_bytes = no_bytes + nbytes_written;   //Total number of bytes written so far
        buf = buf + nbytes_written; //Updating the message pointer in case of partial writes
    }
    return no_bytes;    //Returning the number of bytes written
}

//readline() reads the message (buffer) from the socket file descriptor and returns the number of bytes read or a -1 on error. It also checks for EINTR and calls the function again. It also takes care of EOF and '\n'.
int readline(int sockfd, char *buffer, int max_size_line)
{
    int no_bytes, nchar_read;   //no_bytes corresponds to total number of bytes read and nchar_read gives the number of characters read during each iteration.
    char ch;    //Character read
    
    for(no_bytes = 1; no_bytes < max_size_line; no_bytes++)
    {
        start:
        if((nchar_read = read(sockfd, &ch, 1)) == 1)    //Reading the character and proceeding only in the absence of error or EOF
        {
            *buffer++ = ch; //Storing the character and updating the message pointer
            if(ch == '\n')
                break;
        }
		else if(errno == EINTR) //Recalls the function when an EINTR occurs
        {
            goto start;
        }
        else if(nchar_read == 0 && no_bytes == 1)   //Returns 0 bytes read when the EOF is encountered initially
        {
            return 0;
        }
		else if(nchar_read == 0 )   //When EOF is encountered after reading a few character, function execution is stopped and returns the number of bytes read so far
			break;
        
        else
        {
			return(-1); //Returns an error value in case of an error.
        }
    }

    *buffer = 0;    //Null terminating the string
    return no_bytes;    //Returning the total number of bytes read
}

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in rem_serv;
    char input[MAXCHAR+1], output[MAXCHAR+1];
    int sent_bytes, rec_bytes;
	
	bzero(&rem_serv, sizeof(struct sockaddr));  //Initializing the remote server details with zeros
	rem_serv.sin_family = AF_INET;  //Assigning the IP Family value for the remote server
	rem_serv.sin_port = htons(atoi(argv[2]));   //Assigning the port of Remote Server from the user input
	rem_serv.sin_addr.s_addr = inet_addr(argv[1]);  //Assigning the IP Address of the remote server that client wishes to connect from the user input
	
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= ERROR) //Creating a socket for client and checking for error
	{
        err_sys("Socket Error: ");  //Terminates the program in case of an error in socket creation and displays the corresponding message
		exit(-1);
	}
	

	if((connect(sockfd, (struct sockaddr *)&rem_serv, sizeof(struct sockaddr_in))) <= ERROR) //Establishing a connection between server and client and checking for error
	{
		err_sys("Connect Error: "); //Terminates the program in case of an error in establishing connection and displays the corresponding message
        exit(-1);
	}
    
	printf("The Client has been connected successful to the server with port %d and IP %s \n",ntohs(rem_serv.sin_port),inet_ntoa(rem_serv.sin_addr));   //Confirming to the user if a connection is established
	while(1)
	{
        printf("Please enter the message to be sent to Server:\n");
        if(fgets(input, MAXCHAR, stdin)!=NULL)  //Takes input from user and exchanges the data only when the fgets() gives what user has entered
		{
		if(feof(stdin)) //Checks for EOF (Ctrl+D) and stops the data transfer (when EOF is encountered after a message)
			break;
        printf("The character  read from STDIN is %s \n",input);
        sent_bytes = writen(sockfd, input, strlen(input));  //Sending data to server
		rec_bytes = readline(sockfd, output, MAXCHAR);      //Receiving data from the server
        fprintf(stdout,"Echo from Server:\n%s\n", output);
		}
		if(feof(stdin)) { //Checks for EOF (Ctrl+D) and stops the data transfer (when EOF is encountered initially)
			writen(sockfd, "quit\n", strlen("quit\n"));
			break;
		}
    }
				
    close(sockfd);  //Closes the Client Socket
	fprintf(stdout,"Closing client socket\n");    //Displays message to the user regarding the closure of client socket
	return 0;
}

