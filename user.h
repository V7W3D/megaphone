#ifndef USER_H
#define USER_H
#include <arpa/inet.h>

typedef struct user {
    int id;
    char pseudo[10];
    struct user * next;
} user;

typedef user * lusers;

lusers add_user(lusers head, int id, const char * pseudo);
int est_inscrit(lusers head, int id);
char *get_name(lusers users, int id_user);

#endif