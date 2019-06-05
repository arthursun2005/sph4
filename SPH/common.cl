typedef struct Proxy {
    int index;
    int hash;
} Proxy;

inline int hash(int x, int y) {
    return x + (y << 16);
}

inline int imod(int x, int m) {
    return (x % m + m) % m;
}
