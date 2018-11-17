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
#include <iostream>

#define BUFFER_SIZE 2048000
#define cachesize 10


typedef struct request_info {// store the parameters from the client
	char * address;
	char * resource;
	time_t expires;
	char * accessed;
}request_info;


typedef struct cache_node { // Cache entry structure
	request_info req;
	struct cache_node *next;
	struct cache_node *prev;
} cache_node;

class query_cache {
public:
	query_cache();
	void enqueue(request_info request); // return number of pages
	void dequeue(); // remove LRU
	void remove(request_info request);
	int check_cache(request_info request);
	bool is_cache_exist (request_info req);
	int get_size();
	void print_list();
private:
	cache_node *head; // LRU node; head of the linked list 
	int curr_cache_size;
};
