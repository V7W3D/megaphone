#ifndef USER_H
#define USER_H
#include <arpa/inet.h>

typedef struct {
    int id;
    char pseudo[15];
} user;

struct list_users {
    user * u;
    char u_addr[INET_ADDRSTRLEN];
    struct list_users * suiv;
};

typedef struct list_users * lusers;

lusers add_user(lusers head, user * new_user);

#endif