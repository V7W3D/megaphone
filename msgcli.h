#ifndef MSGCLI_H
#define MSGCLI_H

#include <stdint.h>

typedef struct {
    uint8_t codeReq: 5;
    uint16_t id: 11;
} entete; 

typedef struct {
    uint16_t entete;
    uint16_t numfil;
    uint16_t nb;
    uint8_t datalen;
    char data[256];
} msg_fil;

typedef struct {
    uint16_t entete;
    char pseudo[10];
} msg_inscri;


uint16_t compose_entete(uint8_t codeReq, uint16_t id);
void extract_entete(uint16_t e, uint8_t* codeReq, uint16_t* id);
msg_inscri * compose_msg_inscri(uint16_t entete, const char * pseudo);
msg_fil * compose_msg_fil(const char * data, uint16_t codeReq, uint16_t id, uint16_t numfil, uint16_t nb);

#endif