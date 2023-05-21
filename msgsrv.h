#ifndef MSGSRV_H
#define MSGSRV_H

#include <stdint.h>

typedef struct {
    uint16_t entete;
    uint16_t numfil;
    uint16_t nb;
} msg_srv;

typedef struct{
    uint16_t entete;
    uint16_t numfil;
    uint16_t nb;
    uint8_t adr[16];
} msg_srv_fil;

msg_srv * compose_msg_srv(uint16_t entete, uint16_t numfil, uint16_t nb);
msg_srv_fil * compose_msg_srv_fil(uint16_t entete, uint16_t numfil, uint16_t nb, const char * adr);

#endif