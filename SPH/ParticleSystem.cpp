//
//  ParticleSystem.cpp
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "ParticleSystem.hpp"

void ParticleSystem::sortProxies() {
    size_t size = count;
    
    int k = 0;
    
    clSetKernelArg(sorter, 0, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(sorter, 1, sizeof(tempProxies), (void*)&tempProxies);
    clSetKernelArg(sorter, 2, sizeof(k), (void*)&k);
    clSetKernelArg(sorter, 3, sizeof(size), (void*)&size);
    
    assert(clEnqueueTask(queue, sorter, 0, NULL, NULL) == CL_SUCCESS);
    
    k = 8;
    
    clSetKernelArg(sorter, 0, sizeof(tempProxies), (void*)&tempProxies);
    clSetKernelArg(sorter, 1, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(sorter, 2, sizeof(k), (void*)&k);
    
    assert(clEnqueueTask(queue, sorter, 0, NULL, NULL) == CL_SUCCESS);
    
    k = 16;
    
    clSetKernelArg(sorter, 0, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(sorter, 1, sizeof(tempProxies), (void*)&tempProxies);
    clSetKernelArg(sorter, 2, sizeof(k), (void*)&k);
    
    assert(clEnqueueTask(queue, sorter, 0, NULL, NULL) == CL_SUCCESS);
    
    k = 24;
    
    clSetKernelArg(sorter, 0, sizeof(tempProxies), (void*)&tempProxies);
    clSetKernelArg(sorter, 1, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(sorter, 2, sizeof(k), (void*)&k);
    
    assert(clEnqueueTask(queue, sorter, 0, NULL, NULL) == CL_SUCCESS);
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::createProxies() {
    size_t size = count;
    
    clSetKernelArg(hasher, 0, sizeof(positions_cl), (void*)&positions_cl);
    clSetKernelArg(hasher, 1, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(hasher, 2, sizeof(diameter), (void*)&diameter);
    clSetKernelArg(hasher, 3, sizeof(count), (void*)&count);
    
    clEnqueueNDRangeKernel(queue, hasher, 1, NULL, &size, NULL, 0, NULL, NULL);
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::toOffsetList() {
    size_t size = count;
    
    clSetKernelArg(toList, 0, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(toList, 1, sizeof(offsetList), (void*)&offsetList);
    
    clEnqueueNDRangeKernel(queue, toList, 1, NULL, &size, NULL, 0, NULL, NULL);
    
    clFlush(queue);
    clFinish(queue);
}

void ParticleSystem::solve(float dt) {
    size_t size = count;
    
    clSetKernelArg(solver, 0, sizeof(velocities_cl), (void*)&velocities_cl);
    clSetKernelArg(solver, 1, sizeof(dt), (void*)&dt);
    clSetKernelArg(solver, 2, sizeof(gravity), (void*)&gravity);
    clSetKernelArg(solver, 3, sizeof(positions_cl), (void*)&positions_cl);
    clSetKernelArg(solver, 4, sizeof(proxies), (void*)&proxies);
    clSetKernelArg(solver, 5, sizeof(offsetList), (void*)&offsetList);
    clSetKernelArg(solver, 6, sizeof(count), (void*)&count);
    clSetKernelArg(solver, 7, sizeof(diameter), (void*)&diameter);
    clSetKernelArg(solver, 8, sizeof(accelerations), (void*)&accelerations);
    clSetKernelArg(solver, 9, sizeof(weights), (void*)&weights);
    
    assert(clEnqueueNDRangeKernel(queue, solver, 1, NULL, &size, NULL, 0, NULL, NULL) == CL_SUCCESS);
    
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
    adder = create_cl_kernel(context, device, "solver.cl", "adder");
}

void ParticleSystem::destory_cl() {
    clReleaseContext(context);
    
    releaseMemObjs();
    
    clReleaseCommandQueue(queue);
    
    clReleaseKernel(hasher);
    clReleaseKernel(toList);
    clReleaseKernel(sorter);
    clReleaseKernel(solver);
    clReleaseKernel(adder);
}

void ParticleSystem::step(float dt) {
    if(count == 0) return;
    
    createProxies();
    
    sortProxies();
    
    toOffsetList();
    
    solve(dt);
    
    size_t size = count;
    
    clSetKernelArg(adder, 0, sizeof(velocities_cl), (void*)&velocities_cl);
    clSetKernelArg(adder, 1, sizeof(positions_cl), (void*)&positions_cl);
    clSetKernelArg(adder, 2, sizeof(accelerations), (void*)&accelerations);
    clSetKernelArg(adder, 3, sizeof(dt), (void*)&dt);
    clSetKernelArg(adder, 4, sizeof(diameter), (void*)&diameter);
    
    clEnqueueNDRangeKernel(queue, adder, 1, NULL, &size, NULL, 0, NULL, NULL);
    
    clEnqueueReadBuffer(queue, velocities_cl, CL_TRUE, 0, size * sizeof(vec2), velocities, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, positions_cl, CL_TRUE, 0, size * sizeof(vec2), positions, 0, NULL, NULL);
    
    clFlush(queue);
    clFinish(queue);
}
