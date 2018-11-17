#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <time.h>
#include "query_cache.h"

#define PROXY_DEBUG 1
class proxy {
public: 
	proxy (char const* ip, char const* port_num);
	int recv_from_clnt(); //listenfromclient

private:
	// socket params: 
	struct sockaddr_in serv_addr, client_addr;
	struct hostent *server;
	int sock_fd, client_fd;
	int Isrec;
	socklen_t client_len;
	int fdmax, i;
	unsigned int port;
	fd_set fds_all, fds_read;
	int client_count;
	query_cache proxy_cache;
	void rm_space(char * text); // remove all the spaces from the string	
	request_info parse_req(char buffer[]);
	void send_to_clnt(request_info  clnt_req, int client_fd); //Sendtoclient
	void recv_from_srvr(int web_fd, request_info clnt_req, int client_fd, int condition_flag); // listenfrom
	void get_request_page(request_info clnt_req, int client_fd, int condition_flag); // Processgetpage
	char* generate_req_msg (char *host, char *page);
};

