	#include "server.h"

using namespace std;

Server::Server(char* IP, char* portno, int max_clnt0) {
	int yes=1;        // for setsockopt() SO_REUSEADDR, below

    struct addrinfo hints, *ai, *p;
    max_clnt = max_clnt0;
	curr_clnt = new clnt_info[max_clnt+2];
	clnt_count = 0;
	FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use server IP
	int rv = getaddrinfo(IP, portno, &hints, &ai); // pass IP address here, so server can use other IP address
    if (rv != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
	
	for (p = ai; p != NULL; p = p-> ai_next) {
		srvr_fd = socket(p-> ai_family, p-> ai_socktype, p->ai_protocol);
		if(srvr_fd < 0) {
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(srvr_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));	
		
        if (bind(srvr_fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(srvr_fd);
            continue;
        }

        break;
    }
	
	// if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }
	
    freeaddrinfo(ai); // all done with this
	
	// listen
    if (listen(srvr_fd, 5) == -1) {
        perror("listen");
        exit(3);
    }
	
	// accept
	printf("Waiting for connection...\n");
	
	// add the listener to the master set
    FD_SET(srvr_fd, &master);
	
	// keep track of the biggest file descriptor
    fdmax = srvr_fd; // so far, it's this one
    //cout <<"L64: fdmax = " << fdmax<<endl;
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

int Server::readline(int sockfd, char *buffer, int max_size_line) {
    int no_bytes, nchar_read;   //no_bytes corresponds to total number of bytes read and nchar_read gives the number of characters read during each iteration.
    char ch;    //Character read
    
    for(no_bytes = 1; no_bytes < max_size_line; no_bytes++) {
        start:
        if((nchar_read = read(sockfd, &ch, 1)) == 1) { //Reading the character and proceeding only in the absence of error or EOF
        
            *buffer++ = ch; //Storing the character and updating the message pointer
            if(ch == '\n') break;
        }
		else if(errno == EINTR) { //Recalls the function when an EINTR occurs
            goto start;
        }
        else if(nchar_read == 0 && no_bytes == 1) { //Returns 0 bytes read when the EOF is encountered initially
            return 0;
        }
		else if(nchar_read == 0 )   //When EOF is encountered after reading a few character, function execution is stopped and returns the number of bytes read so far
			break;
        
        else {
			return(-1); //Returns an error value in case of an error.
        }
    }

    *buffer = 0;    //Null terminating the sstring
    return no_bytes;    //Returning the total number of bytes read
}

void Server::forward_message_char (char* buffer, int self_fd, int buff_size) {
	// this function forward message to all other clients in the chat room, using char*
    for(int j = 0; j <= fdmax; j++) {
        if (FD_ISSET(j, &master)) {
            // except the sender client itself and server
            if (j != srvr_fd && j != self_fd) {
                if (writen(j, buffer, buff_size) == -1) {
                    perror("send");
                }
            }
        }
    }
}

void Server::forward_message (sbcp_message fwd, int self_fd, clnt_info* clnt_list, int num_of_clnt) {
	// this function forward message to all other clients in the chat room, using sbcp_message
    for(int j = 0; j <= num_of_clnt; j++) {
        int temp_fd = clnt_list[j].fd;
        if (FD_ISSET(temp_fd, &master)) {
            // except the sender client itself and server
            if (temp_fd != self_fd) {
                if (write(temp_fd, (void *)&fwd, sizeof(fwd)) == -1) {
                    perror("Error on forward");
                }
                char buf = '\0'; // send a null message to push the message from queue
				nullchar = sbcp_message(FWD, "\0", "N/A");
                write(temp_fd, (void *)&nullchar, sizeof(nullchar));
                //cout << "sending message to " << temp_fd<<endl;
            }
        }
    }
}


int Server::sbcp(void) {
	struct sockaddr_storage clnt_addr;
	char remoteIP[INET6_ADDRSTRLEN];
	for (int i = 0; i < 3; i++)
		curr_clnt[i].reset();
	while (true) {
		read_fds = master;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		// run through the existing connections looking for data to read
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == srvr_fd) {
					//cout << "=================clnt_count : " << clnt_count << "; max_clnt" << max_clnt <<endl;
					// handle new connections
					socklen_t clnt_len = sizeof clnt_addr;
					int new_clnt_fd = accept(srvr_fd, (struct sockaddr *)&clnt_addr, &clnt_len);			
					// check accept error
					if (new_clnt_fd == -1) {
							perror("ERROR on accept");
					}
					
					// check number of users in the room
					else if (clnt_count >= max_clnt) {
						// NAK(too many users): return to the join client
						// initalize the message
						nak_msg = sbcp_message(NAK, "Too many users in the chat room!", "N/A");
						//printf("nak: %s\n", nak_msg.Attribute[0].sbcp_payload);
						write(new_clnt_fd, (void *) &nak_msg, sizeof(nak_msg));
					    char buf = '\0';
						nullchar = sbcp_message(FWD, "\0", "N/A");
						// send the message to join client
					    write(new_clnt_fd, (void *)&nullchar, sizeof(nullchar));					
					}
					
					else {
						int temp_fdmax = fdmax;
						FD_SET(new_clnt_fd, &master);
						if (new_clnt_fd > fdmax) fdmax = new_clnt_fd;
						if (!is_clnt_exist(new_clnt_fd)) {
							clnt_count++;
							//cout << "Clnt_count: " << clnt_count << endl;
							cout << "New connection from ";
							cout << inet_ntop(clnt_addr.ss_family,
		                        get_in_addr((struct sockaddr*)&clnt_addr),
		                        remoteIP, INET6_ADDRSTRLEN);
							cout << "; Socket: ";
							cout << new_clnt_fd <<endl;
							cout << "Client name: " << curr_clnt[clnt_count -1].username << endl;
							
							//ACK: return to client
							char namelist[512];
							//cout << "clnt_count = " <<clnt_count<<endl;
							for (int i = 0; i < clnt_count; i++) {
								strcat(namelist, curr_clnt[i].username);
								if (i < clnt_count - 1) strcat(namelist, ", ");
								else strcat(namelist, "\n");
								//cout << "name list: "<< namelist <<endl;
								//cout << "username: " << curr_clnt[i].username << endl;
								//if (i != clnt_count - 2) strcat(namelist, ", ");
							}
							
							string ack_userlist(namelist);			
							string ack_clnt_count = to_string(clnt_count);
							ack_msg = sbcp_message(ACK, ack_clnt_count, ack_userlist);
							write(new_clnt_fd, (void *) &ack_msg, sizeof(ack_msg));
						    char buf = '\0';
							nullchar = sbcp_message(FWD, "\0", "N/A");
						    write(new_clnt_fd, (void *)&nullchar, sizeof(nullchar));
							bzero(namelist, 512);
						    //cout <<"L203: fdmax = " << fdmax<<endl;
						    
							// ONLINE: forward
							//cout <<"ONLINE" << string(curr_clnt[clnt_count -1].username) <<endl;
							online_msg = sbcp_message (ONLINE, string(curr_clnt[clnt_count -1].username), "N/A");
							forward_message(online_msg, curr_clnt[clnt_count -1].fd, curr_clnt, clnt_count);
						}
						else {
							// NAK (for existing username): return to client
							string S1 = "Username already exist!";
							string S2 = "N/A";
							nak_msg = sbcp_message(NAK, S1, S2);
							nak_msg.Attribute[0].Length = strlen("Username already exist!");

							//cout << nak_msg.Attribute[0].sbcp_payload <<endl;
							int res = send(new_clnt_fd, (void *) &nak_msg, sizeof(nak_msg),0);
							if (res > 0) {
								cout <<"NAK sent"<<endl;
							}
							fdmax = temp_fdmax;
							FD_CLR(new_clnt_fd, &master); // clear fd
						}
					}
				}
				else { 
					// SEND: from client
					// First read the message from fd into send_msg
					int nbytes = read(i, (sbcp_message*) &send_msg, sizeof send_msg);
					if (nbytes > 0) cout << "Message received!" << endl;
		            if (nbytes <= 0) {
		                // got error or connection closed by client
		                if (nbytes == 0) {
		                    // connection closed
		                    cout << "Socket "<< i << " hung up\n";
		                    
		                    //OFFLINE: forward to every client
		                    for (int k = 0; k < clnt_count; k++) {
		                    	if (curr_clnt[k].fd == i) {
		                    		offline_msg = sbcp_message(OFFLINE, string(curr_clnt[k].username), "N/A");
		                    		forward_message(offline_msg, curr_clnt[k].fd, curr_clnt, clnt_count);
	                		// delete user from the curr_clnt		        
	                				close(curr_clnt[k].fd);
	                				curr_clnt[k].reset();
	                				clnt_count--;
		                		}
	                		}
                		}
						else {
		                    perror("recv");
		                }
		                close(i); // bye!
		                FD_CLR(i, &master); // remove from master set
		            } 
		            else {
		            	//IDLE: time out from client
		            	if (send_msg.Type == IDLE) {
		            		for (int k = 0; k < clnt_count; k++) {
		                    	if (curr_clnt[k].fd == i) {
		            				idle_msg = sbcp_message(IDLE, string(curr_clnt[k].username), "N/A"); 
									//printf("IDLE version = %d \n Type = %d \n Length = %d \n Attribute type = %d \n attr length = %d \n,",idle_msg.Vrsn,idle_msg.Type, idle_msg.Length,idle_msg.Attribute[0].Type, idle_msg.Attribute[0].Length);
		            				forward_message(idle_msg, i, curr_clnt, clnt_count);
	            				}
            				}
        				}
        				else {
				        	// FORWARD: message to every client
							//printf("SND version = %d \n Type = %d \n Length = %d \n Attribute type = %d \n attr length = %d \n,",fwd_msg.Vrsn,fwd_msg.Type, fwd_msg.Length,fwd_msg.Attribute[0].Type, fwd_msg.Attribute[0].Length);
							fwd_msg.Vrsn = 3;
							fwd_msg.Type = FWD;
							fwd_msg.Attribute[0].Type = USERNAME;
							fwd_msg.Attribute[1].Type = MESSAGE;
		                    for (int k = 0; k < clnt_count; k++) {
	                    		if (curr_clnt[k].fd == i) {
									strcpy(fwd_msg.Attribute[0].sbcp_payload, curr_clnt[k].username);
								}
							}
							strcpy(fwd_msg.Attribute[1].sbcp_payload, send_msg.Attribute[0].sbcp_payload);
							
				            forward_message(fwd_msg, i, curr_clnt, clnt_count);
			            }
		            }
		        } // END handle data from client
			}
		}			
	}
}


bool Server::is_clnt_exist(int fd) { // check if this client alread exist
	char temp_name[16];
	// get the JOIN message from file discriptor
	read(fd, (sbcp_message*)&join_msg, sizeof(join_msg));
	for (int i = 0; i < clnt_count; i++) {
		// look for the user name in the list
		if (!strcmp(join_msg.Attribute[0].sbcp_payload, curr_clnt[i].username)) {
			cout << "Client already exists." << endl;
			return true;
		}
	}
	//cout << "is clnt exist: clnt_count = " <<endl;
	strcpy(curr_clnt[clnt_count].username, join_msg.Attribute[0].sbcp_payload);
	curr_clnt[clnt_count].fd = fd;
	curr_clnt[clnt_count].status = 'O'; // online
	curr_clnt[clnt_count].count = clnt_count;
	
	return false;
}


// get sockaddr, IPv4 or IPv6:
void* Server::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int Server::close_port() {	//close port
	delete[] curr_clnt;
	close(srvr_fd);
	return 0; 
}
