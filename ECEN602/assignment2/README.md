#README
## ECEN 602 NETWORK PROGRAMMING ASSIGNMENT #2
### TEAM 15 (Tong Lu, Jyothsna Kurra)

####ROLE OF EACH TEAM MEMBER:

Tong developed the Server.
Jyothsna worked on the Client code and validated the Test Cases.

####WHAT’S INSIDE?

The package contains five files:
1. server.h
2. makefile
3. server_main.cpp
4. server.cpp
5. client.c6. README.docx
7. Report

####INSTRUCTIONS:

1. In order to compile the server code, run the ‘make’.

2. In order to compile the server code, run the ‘make client’.

3. Upon successful compilation, run the server. On the command line, type: ./server_ip server_port max_clients

4. Next, run the client by typing on the command line: ./client username server_ip server_port

5. To clean the executables run the ‘make clean’.

####SOCKET PROGRAMMING:
Socket programming is a way of connecting two nodes on a network to communicate with each
other. One socket (namely, the server) binds itself to an arbitrary port and starts listening for
connection requests on that port whilst the other node (or) socket (namely, the client) reaches
out to it with connection requests. Once connected, both the client and the server exchange
data. When the client is done exchanging the data, it closes its socket. The server has the
ability to connect to multiple clients at the same time and exchange data with each client
separately.

####NETWORK PROTOCOL:
A network protocol gives a set of rules and conventions for communication between network
devices. Network protocols include mechanisms for devices to identify and make connections
with each other across the network, as well as rules that specify how data is packed into
message packets.

####SIMPLE BROADCAST CHAT PROTOCOL (SBCP) DESCRIPTION:
The Simple Broadcast Chat Protocol (SBCP) is a protocol that allows clients to join and leave
a global chat session, view members of the session, and send and receive messages.
For this assignment, we have to implement a simple TCP Broadcast Chat Server and Client.
Essentially, our goal is to come up with a client - server model where:

1. The server should be able to handle multiple clients. There is an upper cap on the number
of clients it can handle.

2. The client could request the server to join a chat session

3. If the server could accommodate the client, it should send an acknowledgement (ACK) to
the client which contains the number of clients present in the chat session at that instance
and the users (excluding the client) that are present in the chat session.

4. If the server is unable to accommodate the client, it should send a NAK to the client stating
the reason for rejection.

5. Once in the chat session, the client could now receive the messages sent by other users
and server.

6. The client could also send messages to other users via the server (chat text is read from
the standard input)

7. The server should forward the chat text received from one client to all the other clients.

8. The server should forward any status message regarding other clients from the server.

9. The client should receive any status message regarding other clients from the server.

10. The client sends an IDLE message informing server that it has been idle for more than 10
seconds.

11. The server informs the other clients about an idle client if it receives an idle message.At any point, if the server or the clients connected to it are not able to understand a received
packet, they should discard it. Additionally, we also need to equip the server to handle multiple
clients.

####CLIENT:

The Client has been developed using C. The client is essentially taking care of following things

1. Connecting to a remote server

2. Initiating a JOIN with server using username supplied on the command line

3. Receiving ACK/NAK from the server depending upon whether the join request was
accepted or rejected by the server

4. Reading user input and sends the same to the server

5. Receiving messages (sent by other clients) from the server

6. Receiving messages from server which informs the status of other clients in the same chat
session

7. Monitoring its (self) idle time and sends an IDLE message to server when it is idle for more
than 10 seconds

Socket was created using the socket() function. The inputs to the function are IP Family,
Socket Type of the socket to be created and the protocol to be used with that socket. It, in turn,
returns the socket file descriptor of the newly created socket. This new socket file descriptor
is used to represent the client. In case of any error encountered during the creation of socket,
the socket file descriptor turns out to be a non-positive value. In case of error, the program is
terminated and the corresponding error message is displayed.

The remote server details are initialized.

Once the client socket is created, it attempts to connect to the remote server. connect() is used
for this purpose. The inputs to connect() are the client socket file descriptor, details of the
remote server to whom the client wishes to connect to, and the size of the remote server
details. It, in turn, returns the status of the connection. If the connection is successfully
established, a value for 0 is returned. In case of any error encountered while establishing
connection, the return value turns out to be -1. In case of error, the program is terminated and
the corresponding error message is displayed.

If connection is successfully established, the server sends a JOIN request. In case of any error
encountered while establishing connection, the return value turns out to be -1. In case of error,
the program is terminated and the corresponding error message is displayed.

Next, the variables used for select() I/O multiplexing are initialized. The client checks whether
it needs to check for idle time or not. If yes, then, it computes the idle time and compares it
with the upper bound. If the idle time is greater than 10 seconds, the client sends an IDLE
message. In case of any error encountered while establishing connection, the return value
turns out to be -1. In case of error, the program is terminated and the corresponding error
message is displayed.

The client also checks for activity on the socket file descriptor and standard input. If the client
gives an input via STDIN, the input is SEND to the server as a chat text. In case of any error
encountered while establishing connection, the return value turns out to be -1. In case of error,
the program is terminated and the corresponding error message is displayed. If the client
receives a message from receiver, it shall parse the packet using the SBCP Message type
and SBCP attribute type and display the corresponding messages.

SERVER:The Server was developed using C++.

The Socket was created the same way as we did in asignment1: socket() for creating socket, bind to
port using bind(), then listen() for the incoming request.
After the steps above, the file descriptors are monitored by select() to any new data input in a
infinite while loop. If a input from file descriptors is detected, then the program will run through all the current file descriptors. If the data input is from the server file descriptor, that means a new client request a connection. For this case, the program first check the number of existing current in the chat, if the number reach the maximum number, it will reject the connection and send a NAK
back to the client with a reason of “Too many users in the room!”. If there is stll room for new user,
the program then check the name of current user. If a duplicated name is found, the program will
reject the connection and send a NAK back to the client with a reason of “Username already exist!”.
If the user-name doesn’t exist, the program then accept the connection and then send an ACK to the
client, and also forward an ONLINE to everyone else in the chat.

If the data input is from a client file descriptor, the program will check the type of the message and
then forward to everyone in the room, like regular message, and idle. If the read return a 0 from, the
program will send a OFFLINE to the everyone else in the chat and then close the corresponding file
descriptor.