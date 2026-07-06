#include <stdio.h>
#include <string.h>

#include "EmiObject/LinkedObjectList.h"
#include "EmiObject/EObject.h"

LinkedObjectList LinkedObjectList_create()
{
    LinkedObjectList newList = {
        .head = NULL,
        .tail = NULL,
        .size = 0
    };
    return newList;
}

EObject* LinkedObjectList_get_index(LinkedObjectList* list, size_t index)
{
    if (list == NULL || index >= list->size)
        return NULL;

    size_t pos = 0;
    EObject* cur = NULL;

    if (index <= list->size / 2)
    {
        cur = list->head;
        while (pos < index)
        {
            cur = cur->_node_next;
            pos++;
        }
    }
    else
    {
        cur = list->tail;
        pos = list->size - 1;
        while (pos > index)
        {
            cur = cur->_node_prev;
            pos--;
        }
    }
    return cur;
}

int LinkedObjectList_append(LinkedObjectList* list, EObject* item)
{
    if (list == NULL)
        return 1;
    list->size++;
    if (list->tail == NULL)
    {
        list->head = item;
    }
    else
    {
        item->_node_prev = list->tail;
        list->tail->_node_next = item;
    }
    list->tail = item;
    return 0;
}

int LinkedObjectList_insert(LinkedObjectList* list, size_t index, EObject* item)
{
    if (list == NULL)
        return 1;
    if (index >= list->size)
        return LinkedObjectList_append(list, item); /* append fallback */
    EObject* target = LinkedObjectList_get_index(list, index);
    if (target == NULL)
        return 1;
    list->size++;
    item->_node_prev = target->_node_prev;
    item->_node_next = target;
    if (target->_node_prev != NULL)
    {
        target->_node_prev->_node_next = item;
    }
    else
    {
        list->head = item;
    }
    target->_node_prev = item;
    return 0;
}

int LinkedObjectList_remove(LinkedObjectList* list, EObject* item)
{
    if (list == NULL || item == NULL)
        return 1;
    EObject* back = item->_node_prev;
    EObject* front = item->_node_next;
    if (back != NULL)
    {
        back->_node_next = front;
    }
    else
    {
        list->head = front;
    }
    if (front != NULL)
    {
        front->_node_prev = back;
    }
    else
    {
        list->tail = back;
    }
    list->size--;
    item->_node_prev = NULL;
    item->_node_next = NULL;
    return 0;
}

int LinkedObjectList_pop(LinkedObjectList* list, size_t index)
{
    EObject* item = LinkedObjectList_get_index(list, index);
    if (item == NULL)
        return 1;
    return LinkedObjectList_remove(list, item);
}

void LinkedObjectList_clear(LinkedObjectList* list)
{
    if (list == NULL)
        return;
    EObject* cur = list->head;
    while (cur)
    {
        EObject* next = cur->_node_next;
        EObject_Destroy(next);
        cur = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}