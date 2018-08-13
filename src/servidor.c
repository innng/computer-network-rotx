#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

// http://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
#include <sys/socket.h>
// http://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html
#include <arpa/inet.h>
// http://pubs.opengroup.org/onlinepubs/009695399/basedefs/netdb.h.html
#include <netdb.h>

extern int errno;

// functions prototypes
void logExit(const char *str);
void* clientThread(void* param);
char* decoder(char const *cipher, uint32_t X);

// struct to pass the necessary to the threads's function
struct data {
    int sock;
};

// main function
int main(int argc, char const *argv[]) {
    if(argc != 2)
        logExit("Incorrect number of parameters!");

    uint32_t port = atoi(argv[1]);

    // structure that saves the ip address
    struct in_addr inaddr;
    // converts string to address
    inet_pton(AF_INET, "127.0.0.1", &inaddr);

    // structure to store address from protocol family
    struct sockaddr_in addr;
    struct sockaddr *addrptr = (struct sockaddr*)&addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = inaddr;

    // socket creation
    int s;
    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        logExit("Problem creating socket!");

    // set a timeout of 15 seconds
    struct timeval tv;
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)) == -1)
        logExit("Problem setting a timeout!");

    // bind a name to a socket
    if(bind(s, addrptr, sizeof(struct sockaddr_in)) == -1)
        logExit("Couldn't bind a name!");

    // listen for socket connection
    if(listen(s, 10) == -1)
        logExit("Couldn't listen!");

    while(1) {
        struct sockaddr_in raddr;
        struct sockaddr *raddrptr = (struct sockaddr*)&raddr;
        socklen_t rlen = sizeof(struct sockaddr_in);

        int r;
        if((r = accept(s, raddrptr, &rlen)) == -1)
            logExit("Client refused!");

        struct data *aux = malloc(sizeof(struct data));
        if(!aux)
            logExit("Couldn't alloc space!");
            aux->sock = r;

        pthread_t tid;
        if(pthread_create(&tid, NULL, clientThread, aux) != 0)
            logExit("Problem creating thread!");
    }

    exit(EXIT_SUCCESS);
}

// exit the program and print a error msg
void logExit(const char *str) {
    if(errno == 0)
        printf("%s\n", str);
    else
        perror(str);
    exit(EXIT_FAILURE);
}

// treat every client as one independent thread
void* clientThread(void *param) {
    pthread_t tid = pthread_self();

    struct data *socket = param;
    int r = socket->sock;

    uint32_t aux;

    // receiving the string's size
    if(recv(r, (char*)&aux, sizeof(uint32_t), 0) < 1)
        logExit("Couldn't receive the string's size!");
    uint32_t len = ntohl(aux);

    // receiving the string
    char *string = malloc(len);
    if(recv(r, string, len, 0) < 1)
        logExit("Couldn't receive the string!");

    // receiving the X
    if(recv(r, (char*)&aux, sizeof(uint32_t), 0) < 1)
        logExit("Couldn't receive the X!");
    uint32_t X = ntohl(aux);

    // decodifying the string
    char *str = decoder(string, X);
    if(str == NULL)
        logExit("Invalid caracter in string!");

    printf("%s\n", str);

    // sending the string decodified
    if(send(r, str, strlen(str), 0) == -1)
        logExit("Couldn't send the string!");

    close(r);
    pthread_exit(NULL);
}

// caesar cipher decoder
char* decoder(char const *cipher, uint32_t X) {
    // calc cipher size
    int len = strlen(cipher);
    // auxiliar char array
    char aux[len];
    // reserve space for the returning cipher
    char *str;
    if(!(str = malloc(len)))
        return NULL;

    int i = 0, letter = 0;
    for(i =0; i < len; i++) {
        // if lower case letter
        if(cipher[i] >= 97 && cipher[i] <= 122)
            letter = cipher[i] - X;
        // if upper case letter
        else if(cipher[i] >= 65 && cipher[i] <= 90)
            letter = cipher[i] + 32 - X;
        // not a letter
        else return NULL;

        // alphabet circular treatment
        if(letter < 97)
            letter = 26 + letter;

        // sabe the codified letter
        aux[i] = letter;
    }

    // copy to final cipher
    strncpy(str, aux, len);
    return str;
}
