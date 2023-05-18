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
    uint8_t * data;
} msg_fil;

typedef struct {
    uint16_t entete;
    char pseudo[10];
} msg_inscri;

#endif