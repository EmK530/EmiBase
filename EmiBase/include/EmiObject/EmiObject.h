#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/ERect.h"
#include "EmiObject/EImage.h"
#include "EmiObject/EText.h"
#include "EmiObject/LinkedObjectList.h"

#ifndef OPAK_VERSION
    #define OPAK_VERSION 2
#endif

extern LinkedObjectList root_objects;
extern bool EmiObject_AutoDraw;

int EmiObject_Init();
void EmiObject_Draw(int screenWidth, int screenHeight);
EObject* EmiObject_FindN(const char* searchPath, size_t len, EObject* target);
EObject* EmiObject_Find(const char* searchPath, EObject* target);
void EmiObject_Deserialize(const char* filePath);
void EmiObject_Wipe();
void EmiObject_SetDrawOffset(EVector2i value);

#ifndef RELEASE
    /*
        For development use only, serializes all currently existing EmiObject instances into a binary file.
        Can be used to create a static set of objects you load with a one-liner at runtime, to reduce code size.
        If a target is provided, only that object and its children will be serialized, instead of the entire workspace.
    */
    void EmiObject_Serialize(EObject* target);
#else
    static inline void EmiObject_Serialize(EObject* target) {}
#endif