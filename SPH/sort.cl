#include "common.cl"

kernel void sort(global Proxy *A, int iK, int iJ) {
    int i = get_global_id(0);
    int ixj = i ^ iJ;
    if(((i&iK) == 0) ^ (i > ixj)) {
        if(A[i].hash >= A[ixj].hash) {
            Proxy tp = A[i];
            A[i] = A[ixj];
            A[ixj] = tp;
        }
    }else{
        if(A[i].hash <= A[ixj].hash) {
            Proxy tp = A[i];
            A[i] = A[ixj];
            A[ixj] = tp;
        }
    }
}
