#include "common.cl"

kernel void solver(global const float2 *A, const float dt, const float2 g, global const float2 *P, global const Proxy* proxies, global const int* list, const int count, const float D, global float2* R) {
    int i = get_global_id(0);
    
    const float2 p = P[i];
    const float2 v = A[i];
    
    int px = p.x / D;
    int py = p.y / D;
    
    float D2 = D * D;
    
    float2 accel = (float2)(0.0f, 0.0f);
    
    int j, hh;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            hh = map(px + x, py + y);
            
            j = list[hh];
            
            if(j < 0) continue;
            while(j < count) {
                Proxy cell = proxies[j];
                
                if(cell.hash != hh) {
                    break;
                }
                
                if(cell.index == i) {
                    ++j;
                    continue;
                }
                
                float2 jp = P[cell.index];
                
                float2 diff = jp - p;
                float ds = diff.x * diff.x + diff.y * diff.y;
                
                if(ds < D2) {
                    float dr = sqrt(ds);
                    float w = 1.0f - dr/D;
                    float2 jv = A[cell.index];
                    float2 vd = jv - v;
                    float2 n = diff / dr;
                    accel -= 80.0f * w * n;
                    accel += vd * 3.0f;
                }
                
                ++j;
            }
        }
    }
    
    R[i] = dt * (accel + g);
}

kernel void adder(global float2 *A, global float2 *B, global const float2* C, const float dt) {
    int i = get_global_id(0);
    A[i] += C[i];
    B[i] += A[i] * dt;
}
