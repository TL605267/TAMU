#include "proxy.h"

using namespace std;
proxy::proxy (char const* ip, char const* port_num) {
	port = atoi(port_num);
	server = gethostbyname(ip);
	if (server == NULL) {
		fprintf(stderr, "ERROR,no such host\n");
		exit(0);
	}
	bzero((char*)&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;//set family
	serv_addr.sin_port = htons(port); //set the port
									  //serv_addr.sin_addr.s_addr = htonl(inet_network(server)); //set the ip address.
									  //serv_addr.sin_addr.s_addr = INADDR_ANY;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	printf("The proxy server is running, ready for connection!\n\n");

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {//Socket Function
		perror("Error: \n");
		exit(2);
	}
	if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {//bind funciton

		perror("Cannot bind to the socket \n");
		exit(3);
	}

	if ((listen(sock_fd, 5)) < 0) {
		perror("ERROR listenning\n");
		exit(4);
	}

	FD_ZERO(&fds_all);
	FD_ZERO(&fds_read);

	FD_SET(sock_fd, &fds_all);
	client_count = 0;
	fdmax = sock_fd;

}

int proxy::recv_from_clnt(){
	while (1) {
		fds_read = fds_all;

		int fd_count = select(fdmax + 1, &fds_read, NULL, NULL, NULL);
		if (fd_count == -1) {
			printf("Error in select call");
			break;
		}
		for (i = 0; i < fdmax + 1; i++) {
			char buffer[BUFFER_SIZE];
			if (FD_ISSET(i, &fds_read)) {
				if (sock_fd == i) { // deal with new client
					client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_len);
					if (client_fd == -1) {
						perror("Error: accept\n");
						exit(1);
					}
					else {
						FD_SET(client_fd, &fds_all); // add to select fds
						//clientfd=client_fd;
						//clientfd++;
						printf("New client connection has been established...\n");
						if (client_fd>fdmax) // if exceed, ++
						{
							fdmax = client_fd;
						}
					}
				}
				else // existing clients
				{
					if ((Isrec = recv(i, buffer, BUFFER_SIZE, 0))>0) { // received
						if (strncmp(buffer, "GET", 3) == 0) {
							printf("Request message:\n%s\n", buffer);
							request_info clnt_req = parse_req(buffer);
							int condition_flag = proxy_cache.check_cache(clnt_req);
							if (condition_flag == 2) // see if the requset page is in cach
								printf("The request page is not founded in the cache!\n");// condition_flag=0 cache have
							get_request_page(clnt_req, i, condition_flag);
						}

					}
				}
			}
		}
	}
	close(sock_fd);
	return 0;

}

request_info proxy::parse_req(char buffer[]) {
	char *token;
	token = strtok(strdup(buffer), "\r\n");
	request_info clnt_req;
	while (token != NULL)
	{
		char *line = new char[strlen(token)];
		line = token;
		if (strncmp(line, "GET", 3) == 0) {
			char * getline = line + 4;
			getline[strlen(getline) - 8] = '\0';
			clnt_req.resource = getline;
		}
		else if (strncmp(line, "Host: ", 5) == 0) clnt_req.address = line + 6;
		token = strtok(NULL, "\r\n");
	}
	rm_space(clnt_req.resource);
	rm_space(clnt_req.address);
	clnt_req.expires = 0; // Default params; modified later
	clnt_req.accessed = new char[10];
	return clnt_req;

}

void proxy::send_to_clnt(request_info  clnt_req, int client_fd) {
	char * file_name = new char[strlen(clnt_req.address) + strlen(clnt_req.resource) + 1];
	char buf[1];
	// convert to a suitable format
	char resource[strlen(clnt_req.resource)];
	int i;
	char str1 = '/';
	char str2 = '_';
	strcpy(file_name, clnt_req.address);
	strcpy(resource, clnt_req.resource);
	printf("RESOURCE:");
	printf(resource);
	printf("\n");
	for (i = 0; i<strlen(resource); i++) {
		if (strncmp(resource + i, &str1, 1) == 0) strncpy(resource + i, &str2, 1);
	}
	strcat(file_name, resource);
	rm_space(file_name);
	FILE *file = fopen(file_name, "rb");

	bool ready_to_send = false;
	char last_char = ' ';
	char last_2nd_char = ' ';
	char last_3rd_char = ' ';
	char last_4th_char = ' ';

	if (file != NULL) {
		while (fread(buf, 1, 1, file)>0) { // make sure http header doesn't send to client
			if (last_4th_char == '\r' && last_3rd_char == '\n'&&last_2nd_char == '\r' && last_char == '\n' && ready_to_send == false) {
				// since there may be another \r\n\r\n at the end of file, make sure this condition_flagement only apply when ready_to_send flag is false
				ready_to_send = true;
				//printf("rn\n");
			}
			if (ready_to_send == true) {
				if (PROXY_DEBUG == 1) {
					//if (buf[0] == '\r') printf("backslash r");
					//printf("%c", buf[0]);
					if ((send(client_fd, buf, 1, 0)) == -1) {
						perror("Serve failed!");
					}
				}
			}
			last_4th_char = last_3rd_char;
			last_3rd_char = last_2nd_char;
			last_2nd_char = last_char;
			last_char = buf[0];
		}
	}
	FD_CLR(client_fd, &fds_all);
	close(client_fd);
}

//listen from the internet server
void proxy::recv_from_srvr(int web_srvr_fd, request_info clnt_req, int client_fd, int condition_flag) {
	char * file_name = new char[strlen(clnt_req.address) + strlen(clnt_req.resource) + 1];
	char resource[strlen(clnt_req.resource)];
	int i;
	char str1 = '/';
	char str2 = '_';
	strcpy(file_name, clnt_req.address);
	strcpy(resource, clnt_req.resource);
	for (i = 0; i<strlen(resource); i++) {
		if (strncmp(resource + i, &str1, 1) == 0) strncpy(resource + i, &str2, 1);
	}
	strcat(file_name, resource);
	rm_space(file_name);
	int nbytes;
	int iswebcode = 0;
	int isexpire = 0;
	char *webcode = NULL;
	char *expires = NULL;
	char *tk1 = NULL;
	char *tk2 = NULL;
	char content[BUFFER_SIZE];
	memset(content, 0, BUFFER_SIZE);
	char tempstr[10];
	sprintf(tempstr, "tempfile%d", client_fd); // Temporary file
	FILE *tempfile = fopen(tempstr, "wb");
	while ((nbytes = recv(web_srvr_fd, content, BUFFER_SIZE, 0)) > 0) {
		//printf("%s", content);	
		if (iswebcode == 0) {
			tk1 = strtok(strdup(content), "\r\n");
			*(tk1 + 12) = '\0';
			webcode = tk1 + 9;
			iswebcode = 1;
		}
		if (isexpire == 0) {
			tk2 = strtok(strdup(content), "\r\n");
			while (tk2 != NULL) {
				if (strncmp(tk2, "Expires: ", 9) == 0) {
					expires = tk2 + 9;
					isexpire = 1;
					break;
				}
				tk2 = strtok(NULL, "\r\n");
			}
		}
		fwrite(content, 1, nbytes, tempfile);
		memset(content, 0, BUFFER_SIZE);
	}

	fclose(tempfile);
	struct tm tim;
	bzero((void *)&tim, sizeof(struct tm));
	printf("Page will expires: %s\n", expires);
	if (expires != NULL && strcmp(expires, "-1") != 0) {
		strptime(expires, "%a, %d %b %Y %H:%M:%S %Z", &tim);
		clnt_req.expires = mktime(&tim);
	}
	else clnt_req.expires = 0;

	close(web_srvr_fd);

	printf("condition_flagus Code from web server: %s\n", webcode);
	if (nbytes < 0) {
		perror("Receive Error!\n");
		exit(0);
	}
	else {
		if (strcmp(webcode, "304") != 0) { // Response webcode not 304
			if (access(file_name, F_OK) != -1) if (remove(file_name) != 0) printf("Cache: File delete error!\n"); // Save file only if response != 304
			if (rename(tempstr, file_name) != 0) printf("Cache: File cannot be renamed!\n");
		}
		else printf("The requested page has not been changed, sending page from cache of proxy server...\n");
		if (condition_flag == 0 || condition_flag == 2) proxy_cache.remove(clnt_req);
		proxy_cache.enqueue(clnt_req);
		send_to_clnt(clnt_req, client_fd);
		printf("Done\n");
	}
}

// creat a socket to connecet to the web server and get webpage
void proxy::get_request_page(request_info clnt_req, int client_fd, int condition_flag) {
	int rv_fd, web_srvr_fd;
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv_fd = getaddrinfo(clnt_req.address, "80", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_fd));
		exit(1);
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((web_srvr_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("creat socket error");
			continue;
		}
		if (connect(web_srvr_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(web_srvr_fd);
			perror("client connect error");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	//prepare the address and request to send to the webserver
	char *resource = clnt_req.resource;
	if (resource == NULL)
		strcat(resource, "");
	else if (*resource == '/')
		resource++;
	char * Msg;

	printf("condition_flagus: %d\n", condition_flag);
	if (condition_flag == 0 && proxy_cache.is_cache_exist(clnt_req)) {
		printf("Page was in cache, sending to client from cache\n");
		send_to_clnt(clnt_req, client_fd);
		printf("Done\n");

	}
	else if (condition_flag == 2) {
		printf("GET page from the webserver\n\n");
		char temp[] = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
		Msg = (char *)malloc(strlen(temp) - 5 + strlen(clnt_req.address) + strlen(resource) + strlen("ECEN602"));
		sprintf(Msg, temp, resource, clnt_req.address, "ECEN602");
		if ((send(web_srvr_fd, Msg, strlen(Msg), 0)) == -1) {
			perror("GET Failed!");
			exit(1);
		}
		// receive from the wen server
		else recv_from_srvr(web_srvr_fd, clnt_req, client_fd, condition_flag);

	}
	else {//conditional Get
		  //      printf("Conditional GET\n\n");
		//char * cg_file_name = new char[strlen(req.address) + strlen(req.resource) + 1];
		//FILE *cg_file = fopen(cg_file_name, "rb");
		//char* cg_time_buf = new char[1];
		//while ((read = getline(&line, &len, fp)) != -1) {
	    //    printf("Retrieved line of length %zu :\n", read);
	    //    printf("%s", line);
    	//}

		time_t now;
		time(&now);
		struct tm *tim = gmtime(&now);
		char * time_buf = new char[100];
		strftime(time_buf, 80, "%a, %d %b %Y %H:%M:%S %Z", tim);
		char temp[] = "GET /%s HTTP/1.0\r\nHost: %s\r\nIf-Modified-Since: %s\r\n\r\n";
		Msg = (char *)malloc(strlen(temp)+strlen(clnt_req.address)+strlen(resource)+strlen(time_buf));
		sprintf(Msg, temp, resource, clnt_req.address, time_buf);
		printf("Generating conditional get message: \n");
		printf("%s", Msg);
		if ((send(web_srvr_fd, Msg, strlen(Msg), 0)) == -1) {
			perror("GET Failed!");
			exit(1);
		}
		// recieve from the webserver
		else recv_from_srvr(web_srvr_fd, clnt_req, client_fd, condition_flag);
	}
}

void proxy::rm_space(char * text) { // remove all the spaces from the string
	char* m = text;
	char* n = text;
	while (*n != 0) {
		*m = *n++;
		if (*m != ' ') m++;
	}
	*m = 0;
}

char* proxy::generate_req_msg (char *host, char *page) {
  char *pg = page;
  char *content = "GET /%s HTTP/1.0\r\nHost: %s\r\n";
  char* msg = (char *)malloc(strlen(host)+strlen(pg)+strlen(content)-5);
  sprintf(msg, content, pg, host);
  printf("%s", msg);
  return msg;
}