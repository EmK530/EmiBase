#pragma once

#ifndef RELEASE
    int NuklearUI_Init();
    void NuklearUI_Draw();
    extern bool nk_workEarly;
    extern bool nk_emiObject;
    extern bool nk_workLate;
    extern bool nk_postProcess;
    extern bool nk_overlay;
#else
    static inline int NuklearUI_Init() { return 1 };
    static inline void NuklearUI_Draw();
    #define nk_workEarly 0
    #define nk_emiObject 0
    #define nk_workLate 0
    #define nk_postProcess 0
    #define nk_overlay 0
#endif