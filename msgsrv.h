#ifndef MSGSRV_H
#define MSGSRV_H

#include <stdint.h>

typedef struct {
    uint16_t entete;
    uint16_t numfil;
    uint16_t nb;
} msg_srv;

msg_srv * compose_msg_srv(uint16_t entete, uint16_t numfil, uint16_t nb);

#endif