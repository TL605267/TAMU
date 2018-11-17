#include "proxy.h"

#define BUFFER_SIZE 2048000
#define cachesize 10

using namespace std;

int main(int argc, char const *argv[]) {
	if (argc != 3) {
		printf("Usage Syntax: \n\t./proxy <IP TO BIND> <PORT TO BIND>\n");
		exit(1);
	}

	proxy proxy_server(argv[1], argv[2]);
	proxy_server.recv_from_clnt();
	return 0;
}