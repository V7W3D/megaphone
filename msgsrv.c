#include "msgsrv.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fil.h>

msg_srv * compose_msg_srv(uint16_t entete, uint16_t numfil, uint16_t nb){
    msg_srv * m = malloc(sizeof(msg_srv));
    m->entete = entete;
    m->numfil = htons(numfil);
    m->nb = htons(nb);
    return m;
}

msg_srv_fil * compose_msg_srv_fil(uint16_t entete, uint16_t numfil, uint16_t nb, const char * adr){
    msg_srv_fil * m = malloc(sizeof(msg_srv_fil));
    m->entete = entete;
    m->numfil = htons(numfil);
    m->nb = htons(nb);
    inet_pton(AF_INET6, adr, m->adr);
    return m;
}