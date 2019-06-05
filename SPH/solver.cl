#include "common.cl"

kernel void solver(global float2 *A, float dt) {
    int i = get_global_id(0);
    A[i].x = 100.0f;
    A[i].y = 0.0f;
}
