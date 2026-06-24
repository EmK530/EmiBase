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

// Draw EObjects recursively
void Recursive_EObject_Draw(EObject* object, ETransform* parent)
{
    ETransform current;

    object->_render(object, parent, &current);

    LinkedList_foreach(object->Children, child)
    {
        EObject* co = (EObject*)child->item;
        Recursive_EObject_Draw(co, &current);
    }
}

void EmiObject_Draw(int screenWidth, int screenHeight)
{
    Vector2 res = { (float)screenWidth, (float)screenHeight };
    ETransform root = {
        .Position = {0.0f, 0.0f},
        .Size     = {res.x, res.y},
        .Rotation = 0.0f,
        .Anchor   = {0.0f, 0.0f}
    };
    LinkedList_foreach(root_objects, obj)
    {
        EObject *object = (EObject *)obj->item;
        Recursive_EObject_Draw(object, &root);
    }
}

#ifndef RELEASE
    void EmiObject_Serialize()
    {
        
    }
#endif