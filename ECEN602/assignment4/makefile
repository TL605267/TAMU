server: server_main.cpp  server.o
	g++ -o server server_main.cpp server.o
client: echo.c
	gcc -o client echo.c
server.o: server.cpp
	g++ -c -o server.o server.cpp
clean: 
	-rm-f server *.o
	-rm-f client *.o
