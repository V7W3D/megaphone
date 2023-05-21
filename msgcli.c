#include "msgcli.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

uint16_t compose_entete(uint8_t codeReq, uint16_t id) {
    uint16_t temp = (id & 0x7FF) | (codeReq << 11);
    uint16_t  e = htons(temp);
    return e;
}

void extract_entete(uint16_t e, uint8_t* codeReq, uint16_t* id) {
    uint16_t r = ntohs(e);
    *codeReq = (r >> 11) & 0x1F;
    *id = r & 0x7FF;
}

msg_inscri * compose_msg_inscri(uint16_t entete, const char * pseudo){
    msg_inscri * m = malloc(sizeof(msg_inscri));
    m->entete = entete;
    strcpy(m->pseudo, pseudo);
    return m;
}

msg_fil * compose_msg_fil(const char * data, uint8_t codeReq, uint16_t id, uint16_t numfil, uint16_t nb){
    size_t data_size;
    if (data) data_size = strlen(data) + 1;
    else data_size = 0;
    size_t message_size = sizeof(msg_fil) + data_size;
    msg_fil * message = malloc(message_size);
    message->entete = compose_entete(codeReq, id);
    message->numfil = htons(numfil);
    message->nb = htons(nb);
    message->datalen = data_size;
    message->data = (char*) (message + sizeof(msg_fil));
    memcpy(message->data, data, data_size);

    return message;
}

msg_fichier* compose_msg_fichier(uint16_t entete, uint16_t numbloc, char *data){
    msg_fichier* msg = malloc(sizeof(msg_fichier));
    msg->entete = entete;
    msg->numbloc = numbloc;
    strcpy(msg->data, data);
    return msg;
}