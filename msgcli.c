#include "msgcli.h"
#include <stdint.h>

uint16_t compose_entete(entete* header, uint8_t codeReq, uint16_t id) {
    uint16_t temp = (id & 0x7FF) | (codeReq << 11);
    uint16_t  e = htons(temp);
    return e;
}

void extract_entete(uint16_t e, uint8_t* codeReq, uint16_t* id) {
    uint16_t r = ntohs(e);
    *codeReq = (r >> 11) & 0x1F;
    *id = r & 0x7FF;
}

msg_inscri * compose_msg_inscri(uint16_t entete, char * pseudo){
    msg_inscri * m = malloc(sizeof(msg_inscri));
    m->entete = entete;
    strcpy(m->pseudo, pseudo);
    return m;
}