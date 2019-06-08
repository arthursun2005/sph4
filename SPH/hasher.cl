#include "common.cl"

kernel void hasher(global const float2 *A, global Proxy *B, const float D, const int count) {
    int i = get_global_id(0);
    B[i].index = i;
    if(i >= count) {
        B[i].hash = MAX_PARTICLE_COUNT;
    }else{
        B[i].hash = map((int)(A[i].x / D), (int)(A[i].y / D));
    }
}
