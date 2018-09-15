#include "server.h"

Server::Server(char* portno) {
	// Creating socket file descriptor
	if ((srvr_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket opening failed");
		exit(1);
	}
	printf("Socket has been opened...\n");
	bzero((char*) &srvr_addr, sizeof(srvr_addr));
	srvr_port = atoi(portno);
	srvr_addr.sin_family = AF_INET;
	srvr_addr.sin_addr.s_addr = INADDR_ANY;
	srvr_addr.sin_port = htons(srvr_port);
	if (bind(srvr_fd, (struct sockaddr *) &srvr_addr, sizeof(srvr_addr)) == -1) {
		perror("Binding failed");
		exit(1);
	}
	printf("Socket has been binded...\n");
	
	// listen
	listen(srvr_fd, 5);
	clnt_len  = sizeof(clnt_addr);
	// accept
	printf("Waiting for connection...\n");
	clnt_len = sizeof(clnt_addr);
}

void Server::handler(int signum) {
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("Handler: child proecess %d disconnected...\n", pid);
	return;
}

int Server::writen(int sockfd, char *buffer, int nbytes) {
	int no_bytes = 0, nbytes_written = 0;   /*no_bytes corresponds to total number of bytes written; nbytes_written corresponds to number of bytes written during a particular write call.*/
    char *buf;
    buf= buffer;
    while(no_bytes < nbytes) {
        if((nbytes_written = write(sockfd, buf, (nbytes - no_bytes))) <=0) {//Writing the message and checking for error
            if(errno == EINTR) {
                continue;   //Recalling the function when an EINTR occurs
            }
            else {
				return(-1); //Returns an error value in case of an error
            }
        }
        no_bytes = no_bytes + nbytes_written;   //Total number of bytes written so far
        buf = buf + nbytes_written; //Updating the message pointer in case of partial writes
    }
    return no_bytes;    //Returning the number of bytes written
}

int Server::echo(void) { // receive message and echo back to client
	int *new_clnt_fd = (int*)malloc(sizeof(int)); // create fd for new client connection
	if (new_clnt_fd == 0) {
		printf("Out of memory!");
		exit(1);
	}
	if((*new_clnt_fd = accept(srvr_fd, (struct sockaddr *) &clnt_addr, &clnt_len)) == -1){
		perror("ERROR on accept");
		exit(1);
	}

	if ((child_pid = fork()) == 0) {
		close(srvr_fd);
		int child_pid_num = getpid();
		printf("This is child %d\n", child_pid_num);
		while(1) {
			// read 
			bzero(buffer,256);

			if (read(*new_clnt_fd, buffer,255) == -1) {
				perror("ERROR reading from socket");
				exit(1);
			}
			
			//FILE *client_in = fopen((const char*)new_clnt_fd, "r");
			//if (feof(client_in)) {
			if (strcmp(buffer, "quit\n") == 0) {
				printf("Client[%d] is disconnected...\n", child_pid_num);
				//fclose(client_in);
				return 2;
			}  
			// write 
			printf("Client[%d] sent the message: %s\n",child_pid_num, buffer);
			if (writen(*new_clnt_fd, buffer, strlen(buffer)) == -1) {
				perror("ERROR writing to socket");
				exit(1);
			}
		}
	}
	close(*new_clnt_fd);
	free(new_clnt_fd);
	return 0;
}	

int Server::close_port() {	//close port
	close(srvr_fd);
	return 0; 
}

