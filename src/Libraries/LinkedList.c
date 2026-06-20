#include "raylib.h"
#include "Libraries/LinkedList.h"
#include <stdio.h>
#include <string.h>

LinkedList* LinkedList_create()
{
    LinkedList* newList = MemAlloc(sizeof(LinkedList)); /* zeroed allocation, just what we need */
    memset(newList, 0, sizeof(LinkedList));
    if (!newList)
    {
        printf("[LinkedList] Allocation error in LinkedList_create\n");
        return NULL;
    }
    return newList;
}

Node* LinkedList_get_index(LinkedList* list, size_t index)
{
    if (list == NULL || index >= list->size)
        return NULL;

    size_t pos = 0;
    Node* cur = NULL;

    if (index <= list->size / 2)
    {
        cur = list->head;
        while (pos < index)
        {
            cur = cur->front;
            pos++;
        }
    }
    else
    {
        cur = list->tail;
        pos = list->size - 1;
        while (pos > index)
        {
            cur = cur->back;
            pos--;
        }
    }
    return cur;
}

Node* LinkedList_find(LinkedList* list, void* item)
{
    if (list == NULL)
        return NULL;

    for (Node* node = (list)->head; node != NULL; node = node->front)
    {
        if (node->item == item)
        {
            return node;
        }
    }

    return NULL;
}

int LinkedList_append(LinkedList* list, void* item)
{
    if (list == NULL)
        return 1;
    Node* newNode = MemAlloc(sizeof(Node));
    memset(newNode, 0, sizeof(Node));
    if (newNode == NULL)
        return 1;

    newNode->item = item;
    list->size++;

    if (list->tail == NULL)
    {
        list->head = newNode;
    }
    else
    {
        newNode->back = list->tail;
        list->tail->front = newNode;
    }
    list->tail = newNode;

    return 0;
}

int LinkedList_insert(LinkedList* list, size_t index, void* item)
{
    if (list == NULL)
        return 1;
    if (index >= list->size)
        return LinkedList_append(list, item); /* append fallback */
    Node* target = LinkedList_get_index(list, index);
    if (target == NULL)
        return 1;

    Node* newNode = MemAlloc(sizeof(Node));
    memset(newNode, 0, sizeof(Node));
    if (newNode == NULL)
        return 1;

    newNode->item = item;
    list->size++;

    newNode->back = target->back;
    newNode->front = target;

    if (target->back != NULL)
    {
        target->back->front = newNode;
    }
    else
    {
        list->head = newNode;
    }
    target->back = newNode;

    return 0;
}

int LinkedList_remove(LinkedList* list, Node* item, void (*free_function)(void*))
{
    if (list == NULL || item == NULL)
        return 1;

    Node* back = item->back;
    Node* front = item->front;

    if (back != NULL)
    {
        back->front = front;
    }
    else
    {
        list->head = front;
    }

    if (front != NULL)
    {
        front->back = back;
    }
    else
    {
        list->tail = back;
    }

    list->size--;

    item->back = NULL;
    item->front = NULL;

    if (free_function != NULL)
    {
        free_function(item->item);
    }

    MemFree(item);

    return 0;
}

int LinkedList_pop(LinkedList* list, size_t index, void (*free_function)(void*))
{
    Node* item = LinkedList_get_index(list, index);
    if (item == NULL)
        return 1;
    return LinkedList_remove(list, item, free_function);
}

void LinkedList_clear(LinkedList* list, void (*free_function)(void*))
{
    if (list == NULL)
        return;
    Node* cur = list->head;
    while (cur)
    {
        Node* next = cur->front;
        if (free_function != NULL)
        {
            free_function(cur->item);
        }
        MemFree(cur);
        cur = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}
void LinkedList_dispose(LinkedList** list, void (*free_function)(void*))
{
    LinkedList_clear(*list, free_function);
    MemFree(*list);
    *list = NULL;
}