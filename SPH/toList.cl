#include "common.cl"

kernel void toList(global const Proxy *A, global int *B) {
    int i = get_global_id(0);
    Proxy p = A[i];
    if(i == 0 || A[i - 1].hash != p.hash)
        B[p.hash] = i;
}
