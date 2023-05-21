#include <stdlib.h>
#include <string.h>
#include "user.h"

lusers add_user(lusers head, int id, const char * pseudo){
    lusers updated_head = malloc(sizeof(user));
    updated_head->id = id;
    strcpy(updated_head->pseudo, pseudo);
    updated_head->next = head;
    return updated_head;
}

int est_inscrit(lusers head, int id){
    while (head){
        if (head->id == id) return 1;
        head = head->next;
    }
    return 0;
}

char *get_name(lusers users, int id_user){
    while (users){
        if (id_user == users->id) return users->pseudo;
        users = users->next;
    }
    return NULL;
}