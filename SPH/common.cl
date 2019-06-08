#include "settings.h"

#ifndef common_cl
#define common_cl

#define BOUNDS 1

typedef struct Proxy {
    int index;
    int hash;
} Proxy;

inline int imod(int x, int m) {
    return ((x % m) + m) % m;
}

inline int map(int x, int y) {
    return imod(x + (y << 10), MAX_PARTICLE_COUNT);
}

#endif // common_cl
