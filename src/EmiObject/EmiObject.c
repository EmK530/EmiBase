#include <stdio.h>
#include "EmiObject/EmiObject.h"

LinkedList* root_objects;

int EmiObject_Init()
{
    root_objects = LinkedList_create();
    if(root_objects == NULL)
        return 0;

    return 1;
}