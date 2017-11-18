#include "server.h"

int main(int argc, char* argv[]) {
	if (argc < 4) {
		perror("Not enough argument!");
		exit(1);
	}
	Server server(argv[1], argv[2], atoi(argv[3]));
	
	server.sbcp();
	
	server.close_port();
	return 0;
}
