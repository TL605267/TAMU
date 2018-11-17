all: server client
server: proxy_main.cpp proxy.o query_cache.o
	g++ -Wall -o server proxy_main.cpp proxy.o query_cache.o
proxy.o: proxy.cpp query_cache.o
	g++ -c -Wall -o proxy.o proxy.cpp query_cache.o
query_cache.o: query_cache.cpp
	g++ -c -Wall -o query_cache.o query_cache.cpp
client: client.c
	gcc -Wall -o client client.c
clean:
	rm -f server *.o
	rm -f client *.o
