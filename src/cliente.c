#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

extern int errno;

void logexit(const char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

char* decoder(const char *cypher, int X) {
	
	return NULL;
}

int main() {
	printf("heeeeey\n");
	return 0;
}