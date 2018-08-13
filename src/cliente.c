#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>

// http://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
#include <sys/socket.h>
// http://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html
#include <arpa/inet.h>
// http://pubs.opengroup.org/onlinepubs/009695399/basedefs/netdb.h.html
#include <netdb.h>

extern int errno;

// functions prototypes
void logExit(char const *str);
char* encoder(char const *string, uint32_t X);

// main function
int main(int argc, char const *argv[]) {
    if(argc != 5)
        logExit("Incorrect number of parameters");

    // save parameters passed by arguments
    char const *ip = argv[1];
    uint32_t port = atoi(argv[2]);
    char const *string = argv[3];
    uint32_t X = atoi(argv[4]);

    // structure that saves the ip address
    struct in_addr inaddr;
    // converts string to address
    inet_pton(AF_INET, ip, &inaddr);

    // structure to store address from protocol family
    struct sockaddr_in addr;
    struct sockaddr *addrptr = (struct sockaddr*)&addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = inaddr;

    // socket creation
    int s;
    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        logExit("Problem creating socket");

    // set a timeout of 15 seconds
    struct timeval tv;
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)) == -1)
        logExit("Problem setting a timeout");

    // try connecting to a socket on server's side
    if(connect(s, addrptr, sizeof(struct sockaddr_in)) == -1)
        logExit("Problem connecting to the server");

    // calculating string's size
    uint32_t sz = htonl(strlen(string));

    // sending the size of the string
    if(send(s, (char*)&sz, sizeof(uint32_t), 0) == -1)
        logExit("Problem seding the string's size");

    // codifying string
    char const *str = encoder(string, X);
    if(!str)
        logExit("Invalid caracter in string");

    // sending the string
    if(send(s, str, strlen(str), 0) == -1)
        logExit("Couldn't send the string");

    // sending the X
    uint32_t aux = htonl(X);
    if(send(s, (char*)&aux, sizeof(uint32_t), 0) == -1)
        logExit("Couldn't send the X");

    // receiving the string decodified
    char *received = malloc(strlen(str));
    if(recv(s, received, strlen(str), 0) != strlen(str))
        logExit("Couldn't receive the string");

    printf("%s\n", received);

    close(s);
    exit(EXIT_SUCCESS);
}

// exit the program and print a error msg
void logExit(char const *str) {
    if(errno == 0)
        printf("%s\n", str);
    else
        perror(str);

    exit(EXIT_FAILURE);
}

// caesar cipher encoder
char* encoder(char const *string, uint32_t X) {
    // calc string size
    int len = strlen(string);
    // auxiliar char array
    char aux[len];
    // reserve space for the returning string
    char *cipher;
    if(!(cipher = malloc(len)))
        return NULL;

    int i = 0, letter = 0;
    for(i =0; i < len; i++) {
        // if lower case letter
        if(string[i] >= 97 && string[i] <= 122)
            letter = string[i] + X;
        // if upper case letter
        else if(string[i] >= 65 && string[i] <= 90)
            letter = string[i] + 32 + X;
        // not a letter
        else return NULL;

        // alphabet circular treatment
        if(letter > 122)
            letter -= 26;

        // sabe the codified letter
        aux[i] = letter;
    }

    // copy to final string
    strncpy(cipher, aux, len);
    return cipher;
}
