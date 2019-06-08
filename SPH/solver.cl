#include "common.cl"

kernel void solver(global const float2 *A, const float dt, const float2 g, global const float2 *P, global const Proxy* proxies, global const int* list, const int count, const float D, global float2* R, global float* weights) {
    int i = get_global_id(0);
    
    const float2 p = P[i];
    const float2 v = A[i];
    
    int px = (int)(p.x / D);
    int py = (int)(p.y / D);
    
    const float D2 = D * D;
    
    const float cv2 = D2 / (dt * dt);
    const float mp = cv2 * 0.25f;
    
    int j, hh;
    float2 jp, diff, jv, vd, n;
    float ds, dr, w, h;
    
    float weight = 0.0f;
    
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
                
                jp = P[cell.index];
                
                diff = jp - p;
                ds = diff.x * diff.x + diff.y * diff.y;
                if(ds < D2) {
                    dr = sqrt(ds);
                    weight += 1.0f - dr/D;
                }
                ++j;
            }
        }
    }
    
    weight = min(mp, 0.05f * max(weight - 1.0f, 0.0f));
    
    weights[i] = weight;
    
    barrier(CLK_GLOBAL_MEM_FENCE);
    
    float2 accel = (float2)(0.0f, 0.0f);
    
    float qd = dt / D;
    
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
                
                jp = P[cell.index];
                
                diff = jp - p;
                ds = diff.x * diff.x + diff.y * diff.y;
                if(ds < D2) {
                    dr = sqrt(ds);
                    w = 1.0f - dr/D;
                    h = weights[cell.index] + weight;
                    jv = A[cell.index];
                    vd = jv - v;
                    n = diff / dr;
                    accel -= (64.0f * w * h / D) * n;
                    
                    float vn = dot(vd, n);
                    
                    if(vn < 0.0f)
                        accel += (0.25f * max(w, min(-qd * vn, 0.5f)) * vn / dt) * n;
                }
                
                ++j;
            }
        }
    }
    
    R[i] = dt * (accel + g);
}

kernel void adder(global float2 *A, global float2 *B, global const float2* C, const float dt, const float D) {
    int i = get_global_id(0);
    A[i] += C[i];
    
    const float D2 = D * D;
    
    const float cv2 = D2 / (dt * dt);
    
    float v2 = dot(A[i], A[i]);
    if(v2 > cv2) {
        A[i] *= sqrt(cv2 / v2);
    }
    
    B[i] += A[i] * dt;
    
    float2 lowerBound = (float2)(-5.0f, -3.0f);
    float2 upperBound = (float2)(5.0f, 3.0f);
    
#if BOUNDS
    if(B[i].x < lowerBound.x) {
        A[i].x = 0.0f;
        B[i].x = lowerBound.x;
    }
    
    if(B[i].y < lowerBound.y) {
        A[i].y = 0.0f;
        B[i].y = lowerBound.y;
    }
    
    if(B[i].x > upperBound.x) {
        A[i].x = 0.0f;
        B[i].x = upperBound.x;
    }
    
    if(B[i].y > upperBound.y) {
        A[i].y = 0.0f;
        B[i].y = upperBound.y;
    }
#endif
}
