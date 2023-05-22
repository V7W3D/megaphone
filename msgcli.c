#include "msgcli.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
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

msg_fichier* compose_msg_fichier(uint16_t entete, uint16_t numbloc, char *data){
    msg_fichier* msg = malloc(sizeof(msg_fichier));
    msg->entete = entete;
    msg->numbloc = numbloc;
    strcpy(msg->data, data);
    return msg;
}

void send_empty_buffer(struct sockaddr_in6 servadrfichier, uint8_t codeReq
                , uint16_t id, int sock){
    int len = sizeof(msg_fichier);
    msg_fichier *msg = compose_msg_fichier(compose_entete(codeReq, id), htons(-1), "");
    char send_buffer[len];
    memcpy(send_buffer, msg, len);

        //envoyer la requete au serveur
    if (sendto(sock, send_buffer, len, 0,
                    (struct sockaddr*)&servadrfichier, sizeof(servadrfichier)) < 0){
        perror("sendto()");
        exit(EXIT_FAILURE);
    }
}

mes_notification add_notif(mes_notification head, const char * pseudo, const char * data, uint16_t numfil){
    mes_notification m = malloc(sizeof(struct notification));
    strcpy(m->pseudo, pseudo);
    strcpy(m->data, data);
    m->numfil = numfil;
    m->suivant = head;
    return m;
}

void free_notif(mes_notification head){
    mes_notification suiv;
    while(head != NULL){
        suiv = head->suivant;
        free(head);
        head = suiv;
    }
}

void affich_notif(mes_notification head){
    while(head != NULL){
        printf("file : %d pseudo : %s message : %s\n", head->numfil, head->pseudo, head->data);
        head = head->suivant;
    }
}