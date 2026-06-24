#pragma once

#ifndef RELEASE
    int NuklearUI_Init();
    void NuklearUI_Draw();
#else
    static inline int NuklearUI_Init() { return 1 };
    static inline void NuklearUI_Draw();
#endif