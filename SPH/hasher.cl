#include "common.cl"

kernel void hasher(global float2 *A, global Proxy *B, float D, int K, int count) {
    int i = get_global_id(0);
    B[i].index = i;
    
    if(i >= count) {
        B[i].hash = 0x7fffffff;
    }else{
        B[i].hash = imod(hash((int)(A[i].x/D), (int)(A[i].y/D)), K);
    }
}
