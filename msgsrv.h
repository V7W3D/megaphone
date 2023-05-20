#ifndef MSGSRV_H
#define MSGSRV_H

#include <stdint.h>

typedef struct {
    uint16_t entete;
    uint16_t numfil;
    uint16_t nb;
} msg_srv;

typedef struct{
    uint16_t numfil;
    char origin[10];
    char pseudo[10];
    uint8_t datalen;
    char *data;
} msg_dernier_billets;

msg_srv * compose_msg_srv(uint16_t entete, uint16_t numfil, uint16_t nb);
msg_dernier_billets * compose_msg_dernier_billet(uint16_t numfil
                    , char *origin, char *pseudo, uint8_t datalen, char *data);

#endif