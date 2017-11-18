#include <iostream>
#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>

#define STDIN 0
#define VERSION 3
#define JOIN 2
#define SEND 4
#define FWD 3
#define ACK 7
#define NAK 5
#define ONLINE 8
#define OFFLINE 6
#define IDLE 9
#define USERNAME 2
#define MESSAGE 4
#define REASON 1
#define CLIENT_COUNT 3
#define EMPTY 0

using namespace std;

typedef struct clnt_info {
	char username[16];
	int fd;
	char status; // online, idle, or unoccypied
	int count;
	void reset(void) {
		bzero(username, 16);
		fd = -1; // give fd a invalid fd number, so we know it is reseted
		status = 'U'; // U for unoccupied
		count = -1; // give count an invalid number to initalize
	}
	
} clnt_info;

typedef struct sbcp_attribute {
	short int Type; //2(username), 4(message), reason(1), clientcount(3)
	short int Length;
	char sbcp_payload[512];
} sbcp_attribute;

typedef struct sbcp_message {
	uint8_t Vrsn; // 9 bits
	uint8_t Type; // 7 bits 2(join), 4(send), 3(fwd)
    short int Length; // two bytes
	sbcp_attribute Attribute[2];
	sbcp_message(){};
	sbcp_message(short int msg_type, string payload1, string payload2) {
		Vrsn = 3;
		Type = msg_type;
		//cout << "Message type: " << msg_type << endl;
		//cout << "Type: " << Type << endl;
		if (msg_type == JOIN) {
			Attribute[0].Type = USERNAME; // username
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			cout << "JOIN" << " has been created!"<<endl;
		}
		else if (msg_type == SEND) { // for client side only
			Attribute[0].Type = MESSAGE; // message
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			cout << "SEND" << " has been created!"<<endl;			
		}
		else if (msg_type == FWD) {
			Attribute[0].Type = USERNAME; // username (which takes up to 16 bytes)
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			Attribute[1].Type = MESSAGE; // message
			strcpy(Attribute[1].sbcp_payload, payload2.c_str());
			cout << "FWD" << " has been created!"<<endl;
		}
		else if (msg_type == ACK) { // handles up to three user names
			Attribute[0].Type = CLIENT_COUNT; // client count
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			Attribute[1].Type = USERNAME; // username
			strcpy(Attribute[1].sbcp_payload, payload2.c_str());
			cout << "ACK" << " has been created!"<<endl;
		}
		else if (msg_type == NAK) {
			Attribute[0].Type = REASON; // reason
			strncpy(Attribute[0].sbcp_payload, payload1.c_str(), sizeof(Attribute[0].sbcp_payload));
			cout << "NAK" << " has been created!"<<endl;
		}
		else if (msg_type == ONLINE) {
			Attribute[0].Type = USERNAME; // username
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			cout << "ONLINE" << " has been created!"<<endl;
		}
		else if (msg_type == OFFLINE) {
			Attribute[0].Type = USERNAME; // username
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			cout << "OFFLINE" << " has been created!"<<endl;
		}
		 // BE CAREFUL ON THIS!!!!!!!!!!!!!!!!
		else if (msg_type == IDLE) { // server forward to clients
			Attribute[0].Type = USERNAME; // username (which takes up to 16 bytes)
			strcpy(Attribute[0].sbcp_payload, payload1.c_str());
			cout << "IDLE" << " has been created!"<<endl;
		}
		
		else { 
			perror("Invalid argument!\n");
			exit(-1);
			
		}
	}
} sbcp_message;


class Server{
	public:	
		Server(char* IP, char* portno, int max_clnt0);
		int sbcp(void);
		int close_port();	
		int writen(int sockfd, char *buffer, int nbytes);
		int readline(int sockfd, char *buffer, int max_size_line);
	private:
		// socket params: 
		int srvr_fd, clnt_fd;
		struct sockaddr_in srvr_addr;
		char buffer[256];
		string IP;
		// sbcp params & functions: 
		int max_clnt;
		fd_set master;
		fd_set read_fds;
		int fdmax;
		sbcp_message nullchar, join_msg, nak_msg, send_msg, fwd_msg, ack_msg, online_msg, idle_msg, offline_msg;
		clnt_info* curr_clnt;
		int clnt_count;
		
		void forward_message_char (char* buffer, int self_fd, int buff_size);
		void forward_message (sbcp_message fwd, int self_fd, clnt_info* clnt_list, int num_of_clnt);
		bool is_username_used(char n[]); // check whether the username has been occupied
		bool is_clnt_exist(int fd); // check if this client alread exist
		void *get_in_addr(struct sockaddr *sa);		

};
