#include "common.cl"

kernel void sort(global const Proxy *A, global Proxy *B, const int p, const int N) {
    local uint count[0x100];
    
    for(int i = 0; i < 0x100; ++i) {
        count[i] = 0;
    }
    
    for(int i = 0; i < N; ++i) {
        ++count[(A[i].hash >> p) & 0xff];
    }
    
    for(int i = 1; i < 0x100; ++i) {
        count[i] += count[i - 1];
    }
    
    for(int i = N - 1; i != -1; --i) {
        B[--count[(A[i].hash >> p) & 0xff]] = A[i];
    }
}
