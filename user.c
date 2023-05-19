#include <stdlib.h>
#include "user.h"

lusers add_user(lusers head, user * new_user){
    lusers updated_head = malloc(sizeof(struct list_users));
    updated_head->u = new_user;
    updated_head->suiv = head;
    return updated_head;
}