#pragma once

#ifndef RELEASE
    #define PSTR(s) (s)
#else
    #define PSTR_KEY 0x5A // Needs to match tools/pstr_replacer.py

    static inline char* _pstr_decode(char* buf, int len)
    {
        for (int i = 0; i < len; i++)
            buf[i] ^= (PSTR_KEY ^ i);
        return buf;
    }

    // PSTR macro is replaced by the build script in release builds
    #define PSTR(s) (s)
#endif