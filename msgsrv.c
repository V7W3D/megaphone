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

msg_dernier_billets * compose_msg_dernier_billet(uint16_t numfil
                    , char *origin, char *pseudo, uint8_t datalen, char *data){
    msg_dernier_billets * msg = malloc(sizeof(msg_dernier_billets));
    msg->numfil = num_fil;
    strcpy(msg->origin, origin);
    strcpy(msg->pseudo, pseudo);
    msg->datalen = datalen;
    msg->data = malloc(datalen);
    strcpy(msg->data, data);

    return msg;
}