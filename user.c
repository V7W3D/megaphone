#include <stdlib.h>
#include <string.h>
#include "user.h"

lusers add_user(lusers head, int id, const char * pseudo){
    lusers updated_head = malloc(sizeof(user));
    updated_head->id = id;
    if(pseudo != NULL) strcpy(updated_head->pseudo, pseudo);
    updated_head->next = head;
    return updated_head;
}