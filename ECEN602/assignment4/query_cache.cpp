#include "query_cache.h"
using namespace std;
	
query_cache::query_cache(){
	head = NULL; 
	curr_cache_size = 0;
}

void query_cache::enqueue(request_info request) {
	cache_node *curr;
	curr = new cache_node();
	curr->req.address = request.address;
	curr->req.resource = request.resource;
	curr->req.expires = request.expires;
	time_t now;
	time(&now);
	struct tm *tim = gmtime(&now);
	char * buf = new char[100];
	strftime(buf, 80, "%a, %d %b %Y %H:%M:%S %Z", tim);
	printf("Last modified at: %s\n", buf);
	curr->req.accessed = buf; // Make last-accessed time = current time

	if (head == NULL) { // If cache is empty
		curr->next = NULL;
		curr->prev = NULL;
		head = curr;
	}
	else { // Push node to top
		head->prev = curr;
		curr->next = head;
		curr->prev = NULL;
		head = curr;
	}
	curr_cache_size++;
	if (curr_cache_size == cachesize + 1) { // Max limit for the cache is hit; dequeue the LRU entry
		dequeue();
	}
}

void query_cache::dequeue() {
	cache_node* curr;
	if (head == NULL) {
		cout << "Error: empty cache" <<endl;
	}
	else {
		curr = head;
		for (int m = 0; m<cachesize - 1; m++)
			curr = curr->next;
		curr->next = NULL;
		delete curr;
		curr_cache_size--;
		cout << "The LRU page has been dequeued!"<<endl;
	}
}
void query_cache::remove(request_info request) {
	cache_node* curr, *t1, *t2;
	curr = head;
	while (curr != NULL) {
		if (!strcmp(curr->req.address, request.address) && !strcmp(curr->req.resource, request.resource)) {
			t1 = curr->prev;
			t2 = curr->next;
			if (t1 != NULL)t1->next = t2; else head = t2;
			if (t2 != NULL)t2->prev = t1;
			delete curr; // recycle memory
			curr_cache_size--;
			break;
		}
		curr = curr->next;
	}
}

int query_cache::check_cache (request_info request) {
	cache_node* curr, *t1, *t2;
	curr = head;
	while (curr != NULL) {
		if (!strcmp(curr->req.address, request.address) && !strcmp(curr->req.resource, request.resource)) { // Cache Hit
			time_t present;
			time(&present); // get the current calendar time
			char * buff = new char[100];
			struct tm *tim = gmtime(&present);
			strftime(buff, 80, "%a, %d %b %Y %H:%M:%S %Z", tim);
			//printf("Current time is: %s\n", buff);

			printf("Request page is founded, ");

			if (mktime(tim) > curr->req.expires) { // Expired
				char * buffer = new char[100];
				time(&curr->req.expires);
				struct tm *tim = gmtime(&curr->req.expires);
				strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S %Z", tim);
				//printf("Lastmodified time is: %s\n", buffer);
				printf("but is already expired...!\n");
				t1 = curr->prev;
				t2 = curr->next;
				if (t1 != NULL)t1->next = t2; else head = t2;
				if (t2 != NULL)t2->prev = t1;
				curr_cache_size--;
				return 1;
			}
			else { // Not expired
				printf("and is not yet expired!\n");
				return 0; // succeed
			}
		}
		else curr = curr->next; // 
	}
	return 2; // Cache Miss
}


bool query_cache::is_cache_exist (request_info req) {
	cache_node *curr = head;
	while (curr != NULL) { // Finding last-accessed time for the requested page in cache
		if (!strcmp(curr->req.address, req.address) && !strcmp(curr->req.resource, req.resource)) return true;
		curr = curr->next;
	}
	return false;
}

int query_cache::get_size() {
	return curr_cache_size;
}

void print_list(){

}