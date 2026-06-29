#pragma once

#include "EObject.h"
#include "ERect.h"
#include "Libraries/LinkedList.h"

extern LinkedList* root_objects;

int EmiObject_Init();
void EmiObject_Draw(int screenWidth, int screenHeight);
void EmiObject_Deserialize(const char* filePath);
void EmiObject_Wipe();
void EmiObject_SetDrawOffset(EVector2i value);

#ifndef RELEASE
    /*
        For development use only, serializes all currently existing EmiObject instances into a binary file.
        Can be used to create a static set of objects you load with a one-liner at runtime, to reduce code size.
    */
    void EmiObject_Serialize();
#endif