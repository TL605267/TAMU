#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h> // exit()
#include <netinet/in.h> // struct sockaddr_in
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

class Server{
	public:
		Server(char* portno);
		int echo();
		int close_port();
		void handler(int signum);
		int writen(int sockfd, char *buffer, int nbytes);
		int readline(int sockfd, char *buffer, int max_size_line);
	private:
		int srvr_port, srvr_fd, clnt_fd;
		struct sockaddr_in srvr_addr, clnt_addr;
		socklen_t clnt_len;
		char buffer[256];
		pid_t child_pid;
};
