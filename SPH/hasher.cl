#include "common.cl"

kernel void hasher(global const float2 *A, global Proxy *B, const float D) {
    int i = get_global_id(0);
    B[i].index = i;
    B[i].hash = map(A[i].x/D, A[i].y/D);
}
