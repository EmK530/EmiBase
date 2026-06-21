#include <stdio.h>
#include "EmiObject/EmiObject.h"

LinkedList* root_objects;

int EmiObject_Init()
{
    root_objects = LinkedList_create();
    if(root_objects == NULL)
        return 0;

    eprintf("[EmiObject] Ready!\n");
    return 1;
}