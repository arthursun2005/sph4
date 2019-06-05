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
#define ParticleSystemInitialCapacity 256
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
    
    cl_context context;
    cl_device_id device;
    
    cl_mem proxies;
    cl_mem offsetList;
    
    cl_mem positions_cl;
    cl_mem velocities_cl;
    
    cl_command_queue queue;
    
    vec2* positions;
    vec2* velocities;
    float* weights;
    
    float diameter;
    
    int capacity;
    int count;
    
    inline void releaseMemObjs() {
        clReleaseMemObject(proxies);
        clReleaseMemObject(offsetList);
        
        clReleaseMemObject(positions_cl);
        clReleaseMemObject(velocities_cl);
    }
    
    inline void createMemObjs() {
        proxies = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(Proxy) * capacity, NULL, NULL);
        offsetList = clCreateBuffer(context, CL_MEM_READ_WRITE, 2 * sizeof(int) * capacity, NULL, NULL);
        
        positions_cl = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(vec2) * capacity, NULL, NULL);
        velocities_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(vec2) * capacity, NULL, NULL);
    }
    
    void growBuffers() {
        int oldCapacity = capacity;
        capacity *= 2;
        
        Realloc(&positions, oldCapacity * sizeof(vec2), capacity * sizeof(vec2));
        Realloc(&velocities, oldCapacity * sizeof(vec2), capacity * sizeof(vec2));
        Realloc(&weights, oldCapacity * sizeof(float), capacity * sizeof(float));
        
        releaseMemObjs();
        createMemObjs();
    }
    
    void initialize_cl();
    
    void destory_cl();
    
    
    
    void createProxies();
    
    void sortProxies();
    
    void toOffsetList();
    
    void solve(float dt);
    
public:
    
    vec2 gravity;
    
    ParticleSystem(const vec2& gravity) : gravity(gravity) {}
    
    ~ParticleSystem() {
        if(capacity > 0)
            this->destory();
        
        destory_cl();
    }
    
    void initialize(float D) {
        capacity = ParticleSystemInitialCapacity;
        count = 0;
        diameter = D;
        
        positions = (vec2*)Alloc(capacity * sizeof(vec2));
        velocities = (vec2*)Alloc(capacity * sizeof(vec2));
        weights = (float*)Alloc(capacity * sizeof(float));
        
        initialize_cl();
    }
    
    void destory() {
        Free(positions);
        Free(velocities);
        Free(weights);
        
        capacity = 0;
    }
    
    inline void addParticle(const vec2& p, const vec2& v) {
        if(count >= capacity)
            growBuffers();
        
        positions[count] = p;
        velocities[count++] = v;
    }
    
    void add(const Shape& shape, const vec2& linearVelocity) {
        int oldCount = count;
        
        AABB aabb = shape.aabb();
        float stride = diameter * DistBtwParticles;
        for (float y = floorf(aabb.lowerBound.y / stride) * stride; y < aabb.upperBound.y; y += stride) {
            for (float x = floorf(aabb.lowerBound.x / stride) * stride; x < aabb.upperBound.x; x += stride) {
                vec2 p(x, y);
                if (shape.includes(p))
                    addParticle(p, linearVelocity);
            }
        }
        
        //clEnqueueWriteBuffer(queue, velocities_cl, CL_TRUE, oldCount * sizeof(vec2), (count - oldCount) * sizeof(vec2), velocities, 0, NULL, NULL);
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
