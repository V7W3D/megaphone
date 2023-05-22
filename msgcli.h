#ifndef MSGCLI_H
#define MSGCLI_H

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 512

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

typedef struct{
    uint16_t entete;
    uint16_t numbloc;
    char data[512];
} msg_fichier;

struct notification {
    uint16_t numfil;
    char pseudo[10];
    char data[20];
    struct notification * suivant; 
};

typedef struct notification * mes_notification;

uint16_t compose_entete(uint8_t codeReq, uint16_t id);
void extract_entete(uint16_t e, uint8_t* codeReq, uint16_t* id);
msg_inscri * compose_msg_inscri(uint16_t entete, const char * pseudo);
msg_fichier* compose_msg_fichier(uint16_t entete, uint16_t numbloc, char *data);
msg_fil * compose_msg_fil(const char * data, uint8_t codeReq, uint16_t id, uint16_t numfil, uint16_t nb);
void send_empty_buffer(struct sockaddr_in6 servadrfichier, uint8_t codeReq, uint16_t id, int sock);
mes_notification add_notif(mes_notification head, const char * pseudo, const char * data, uint16_t numfil);
void free_notif(mes_notification head);
void affich_notif(mes_notification head);
                    

#endif