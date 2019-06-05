//
//  ParticleSystem.cpp
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "ParticleSystem.hpp"

#define ParticleSystemCLGroupSize 1024

void ParticleSystem::sortProxies() {
    size_t size = capacity;
    size_t group = ParticleSystemCLGroupSize;
    
    clSetKernelArg(sorter, 0, sizeof(proxies), (void*)&proxies);
    
    for(cl_int k = 2; k <= size; k <<= 1) {
        clSetKernelArg(sorter, 1, sizeof(k), (void*)&k);
        for(cl_int j = k >> 1; j > 0; j >>= 1) {
            clSetKernelArg(sorter, 2, sizeof(j), (void*)&j);
            clEnqueueNDRangeKernel(queue, sorter, 1, NULL, &size, &group, 0, NULL, NULL);
        }
    }
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::createProxies() {
    size_t size = capacity;
    size_t group = ParticleSystemCLGroupSize;
    
    int K = 2 * capacity;
    
    clSetKernelArg(hasher, 0, sizeof(positions_cl), (void*)&positions_cl);
    clSetKernelArg(hasher, 1, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(hasher, 2, sizeof(diameter), (void*)&diameter);
    clSetKernelArg(hasher, 3, sizeof(K), (void*)&K);
    clSetKernelArg(hasher, 4, sizeof(count), (void*)&count);
    
    clEnqueueNDRangeKernel(queue, hasher, 1, NULL, &size, &group, 0, NULL, NULL);
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::toOffsetList() {
    size_t size = count;
    size_t group = ParticleSystemCLGroupSize;
    
    clSetKernelArg(toList, 0, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(toList, 1, sizeof(offsetList), (void*)&offsetList);
    
    clEnqueueNDRangeKernel(queue, toList, 1, NULL, &size, &group, 0, NULL, NULL);
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::solve(float dt) {
    size_t size = count;
    size_t group = ParticleSystemCLGroupSize;
    
    clSetKernelArg(solver, 0, sizeof(velocities_cl), (void*)&velocities_cl);
    clSetKernelArg(solver, 1, sizeof(dt), (void*)&dt);
    
    clEnqueueNDRangeKernel(queue, solver, 1, NULL, &size, &group, 0, NULL, NULL);
    
    clEnqueueReadBuffer(queue, velocities_cl, CL_TRUE, 0, size * sizeof(vec2), velocities, 0, NULL, NULL);
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::initialize_cl() {
    context = create_cl_context(CL_DEVICE_TYPE_CPU, &device);
    
    createMemObjs();
    
    queue = clCreateCommandQueue(context, device, 0, NULL);
    
    hasher = create_cl_kernel(context, device, "hasher.cl", "hasher");
    toList = create_cl_kernel(context, device, "toList.cl", "toList");
    sorter = create_cl_kernel(context, device, "sort.cl", "sort");
    solver = create_cl_kernel(context, device, "solver.cl", "solver");
}

void ParticleSystem::destory_cl() {
    clReleaseContext(context);
    
    releaseMemObjs();
    
    clReleaseCommandQueue(queue);
    
    clReleaseKernel(hasher);
    clReleaseKernel(toList);
    clReleaseKernel(sorter);
    clReleaseKernel(solver);
}

void ParticleSystem::step(float dt) {
    //createProxies();
    //sortProxies();
    //toOffsetList();
    
    solve(dt);
    
    vec2 gforce = dt * gravity;
    for (int i = 0; i < count; ++i) {
        //velocities[i] += gforce;
    }
    
    for (int i = 0; i < count; ++i) {
        positions[i] += dt * velocities[i];
    }
}
