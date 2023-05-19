#include "msgsrv.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

msg_srv * compose_msg_srv(uint16_t entete, uint16_t numfil, uint16_t nb){
    msg_srv * m = malloc(sizeof(msg_srv));
    m->entete = entete;
    m->numfil = htons(numfil);
    m->nb = htons(nb);
    return m;
}