#pragma once

#include <stddef.h>

typedef struct EObject EObject;

typedef struct {
  EObject* head;
  EObject* tail;
  size_t size;
} LinkedObjectList;

#define LinkedObjectList_foreach(list, object) \
    for (EObject* object = (list).head; object != NULL; object = object->_node_next)

LinkedObjectList LinkedObjectList_create();

EObject* LinkedObjectList_get_index(LinkedObjectList* list, size_t index);

int LinkedObjectList_insert(LinkedObjectList* list, size_t index, EObject* item);

int LinkedObjectList_append(LinkedObjectList* list, EObject* item);

int LinkedObjectList_remove(LinkedObjectList* list, EObject* item);

int LinkedObjectList_pop(LinkedObjectList* list, size_t index);

void LinkedObjectList_clear(LinkedObjectList* list);