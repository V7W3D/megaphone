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

msg_fil * compose_msg_fil(const char * data, uint16_t codeReq, uint16_t id, uint16_t numfil, uint16_t nb){
    size_t data_size = 0;
    if(data != NULL){
        data_size = strlen(data) + 1;
    }
    msg_fil * message = malloc(sizeof(msg_fil));
    message->entete = compose_entete(codeReq, id);
    message->numfil = htons(numfil);
    message->nb = htons(nb);
    if(data != NULL){
        message->datalen = data_size;
        strcpy(message->data, data);
    }
    return message;
}