#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "user.h"
#include "msg.h"

#define BUF_SIZE 256

lusers my_users = NULL;
int id_u = 0;

void register_user(char * pseudo){
    user * u = malloc(sizeof(user));
    u->id = id_u;
    strcpy(u->pseudo, pseudo);
    my_users = add_user(my_users, u);
}

int main(){
    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    struct sockaddr_in6 cliadr;
    memset(&cliadr, 0, sizeof(cliadr));
    cliadr.sin6_family = AF_INET6;
    cliadr.sin6_addr = in6addr_any;
    cliadr.sin6_port = htons(7777);

    if (bind(sock, (struct sockaddr *)&cliadr, sizeof(cliadr)) < 0) return -1;


    char buffer[BUF_SIZE+1];
    struct sockaddr_in6 cliadr;
    socklen_t len = sizeof(cliadr);
}