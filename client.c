#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "user.h"
#include "msgcli.h"

#define BUF_SIZE 256

int main(){
    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_port = htons(7777);
    inet_pton(AF_INET6, "::1", &servadr.sin6_addr);

    char buffer[BUF_SIZE+1];
    struct sockaddr_in6 servadr;
    socklen_t len = sizeof(servadr);
}