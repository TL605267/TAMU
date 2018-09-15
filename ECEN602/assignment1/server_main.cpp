#include "server.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		perror("No port provided!");
		exit(1);
	}
	Server server(argv[1]);
	
	while(1) {
		if (server.echo() == 2) {
			break;
		}
	}
	server.close_port();
	return 0;
}
