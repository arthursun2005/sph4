//
//  ParticleSystem.hpp
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ParticleSystem_hpp
#define ParticleSystem_hpp

#include "common.h"
#include "Shape.h"

#ifndef ParticleSystemInitialCapacity
#define ParticleSystemInitialCapacity 1024
#endif

#ifndef DistBtwParticles
#define DistBtwParticles 0.75f
#endif

struct Proxy
{
    int index;
    int hash;
};

class ParticleSystem
{
    cl_kernel hasher;
    cl_kernel toList;
    cl_kernel sorter;
    cl_kernel solver;
    cl_kernel adder;
    
    cl_context context;
    cl_device_id device;
    
    cl_mem proxies;
    cl_mem tempProxies;
    cl_mem offsetList;
    cl_mem accelerations;
    cl_mem weights;
    
    cl_mem positions_cl;
    cl_mem velocities_cl;
    
    cl_command_queue queue;
    
    vec2 positions[MAX_PARTICLE_COUNT];
    vec2 velocities[MAX_PARTICLE_COUNT];
    
    float diameter;
    
    int count;
    
    inline void releaseMemObjs() {
        clReleaseMemObject(proxies);
        clReleaseMemObject(tempProxies);
        clReleaseMemObject(offsetList);
        clReleaseMemObject(weights);
        
        clReleaseMemObject(positions_cl);
        clReleaseMemObject(velocities_cl);
        clReleaseMemObject(accelerations);
    }
    
    inline void createMemObjs() {
        proxies = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(Proxy) * MAX_PARTICLE_COUNT, NULL, NULL);
        tempProxies = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(Proxy) * MAX_PARTICLE_COUNT, NULL, NULL);
        offsetList = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * MAX_PARTICLE_COUNT, NULL, NULL);
        weights = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * MAX_PARTICLE_COUNT, NULL, NULL);
        
        positions_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(vec2) * MAX_PARTICLE_COUNT, NULL, NULL);
        velocities_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(vec2) * MAX_PARTICLE_COUNT, NULL, NULL);
        accelerations = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(vec2) * MAX_PARTICLE_COUNT, NULL, NULL);
    }
    
    void initialize_cl();
    
    void destory_cl();
    
    
    
    void createProxies();
    
    void sortProxies();
    
    void toOffsetList();
    
    void solve(float dt);
    
public:
    
    vec2 gravity;
    
    inline ParticleSystem(const vec2& gravity) : gravity(gravity) {}
    
    inline ~ParticleSystem() {
        destory_cl();
    }
    
    void initialize(float D) {
        count = 0;
        diameter = D;
        initialize_cl();
    }
    
    inline void clear() {
        count = 0;
        releaseMemObjs();
        createMemObjs();
    }
    
    inline void addParticle(const vec2& p, const vec2& v) {
        if(count < MAX_PARTICLE_COUNT) {
            positions[count] = p;
            velocities[count++] = v;
        }
    }
    
    void add(const Shape& shape, const vec2& linearVelocity, float dist = DistBtwParticles) {
        int oldCount = count;
        
        AABB aabb = shape.aabb();
        float stride = diameter * dist;
        for (float y = aabb.lowerBound.y; y < aabb.upperBound.y; y += stride) {
            for (float x = aabb.lowerBound.x; x < aabb.upperBound.x; x += stride) {
                vec2 p(x, y);
                if (shape.includes(p))
                    addParticle(p, linearVelocity);
            }
        }
        
        clEnqueueWriteBuffer(queue, velocities_cl, CL_TRUE, oldCount * sizeof(vec2), (count - oldCount) * sizeof(vec2), velocities + oldCount, 0, NULL, NULL);
        clEnqueueWriteBuffer(queue, positions_cl, CL_TRUE, oldCount * sizeof(vec2), (count - oldCount) * sizeof(vec2), positions + oldCount, 0, NULL, NULL);
        
        clFlush(queue);
        clFinish(queue);
    }
    
    inline int getCount() const {
        return count;
    }
    
    void step(float dt);
    
    inline void step(float dt, int its) {
        float _dt = dt / (float) its;
        for(int i = 0; i < its; ++i)
            step(_dt);
    }
    
    friend class PSGraphic;
};

#endif /* ParticleSystem_hpp */
