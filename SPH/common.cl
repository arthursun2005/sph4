#include "settings.h"

#ifndef common_cl
#define common_cl

typedef struct Proxy {
    int index;
    int hash;
} Proxy;

inline int hash(int x, int y) {
    return x + (y << 10);
}

inline int imod(int x, int m) {
    return ((x % m) + m) % m;
}

inline int map(int x, int y) {
    return imod(hash(x, y), MAX_PARTICLE_COUNT);
}

#endif // common_cl
