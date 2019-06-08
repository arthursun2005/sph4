//
//  common.h
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef common_h
#define common_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "settings.h"

#define current_nanosecond \
std::chrono::high_resolution_clock::now()

#define current_microsecond \
std::chrono::system_clock::now()

typedef std::chrono::high_resolution_clock::time_point nanosecond_type;

#define Alloc(size) \
(__builtin_operator_new(size))

#define Free(ptr) \
(__builtin_operator_delete(ptr))

#define randf \
(rand() / (float) RAND_MAX)

template <class T>
inline void Realloc(T** ptr, int oldSize, int size) {
    void* oldPtr = *ptr;
    *ptr = (T*)Alloc(size);
    std::memcpy(*ptr, oldPtr, oldSize);
    Free(oldPtr);
}

struct vec2
{
    float x;
    float y;
    
    inline vec2() {}
    
    inline vec2(float x, float y) : x(x), y(y) {}
    
    inline vec2& operator -= (const vec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    
    inline vec2& operator += (const vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    
    inline vec2 operator - () const {
        return vec2(-x, -y);
    }
    
    inline vec2 T () const {
        return vec2(x, -y);
    }
    
    inline vec2 I () const {
        return vec2(y, -x);
    }
    
    inline vec2 norm () const {
        float l = sqrtf(lengthSq());
        return vec2(x/l, y/l);
    }
    
    inline float lengthSq() const {
        return x * x + y * y;
    }
};

inline vec2 operator + (const vec2& a, const vec2& b) {
    return vec2(a.x + b.x, a.y + b.y);
}

inline vec2 operator - (const vec2& a, const vec2& b) {
    return vec2(a.x - b.x, a.y - b.y);
}

inline vec2 operator * (float a, const vec2& b) {
    return vec2(a * b.x, a * b.y);
}

inline vec2 operator * (const vec2& a, const vec2& b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

inline float dot (const vec2& a, const vec2& b) {
    return a.x * b.x + a.y * b.y;
}

struct Frame
{
    int x;
    int y;
    int w;
    int h;
    
    float scl;
    vec2 offset;
    
    inline Frame() {}
    
    inline Frame(int x, int y, int w, int h, float scl, const vec2& offset) : x(x), y(y), w(w), h(h), scl(scl), offset(offset) {}
};

/**
 * most cl_* types are internal pointers
 * so passing them around doesn't matter
 */

inline int fstr(const char* file_name, std::string* str) {
    std::ifstream file;
    file.open(file_name);
    
    if(!file.is_open()) {
        printf("%s cannot be opened\n", file_name);
        return -1;
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    str->assign(ss.str());
    return 0;
}

inline cl_context create_cl_context(cl_int type, cl_device_id* device_id) {
    cl_platform_id platform_id = NULL;
    cl_uint num_platforms, num_devices;
    clGetPlatformIDs(1, &platform_id, &num_platforms);
    clGetDeviceIDs(platform_id, type, 1, device_id, &num_devices);
    return clCreateContext(NULL, 1, device_id, NULL, NULL, NULL);
}

inline cl_program create_cl_program(cl_context context, const char* file_name) {
    std::string str;
    int success = fstr(file_name, &str);
    
    if(success == -1)
        return NULL;
    
    const char* source = str.c_str();
    const size_t size = str.size();
    
    printf("compiled %s\n", file_name);
    
    return clCreateProgramWithSource(context, 1, &source, &size, NULL);
}

inline cl_kernel create_cl_kernel(cl_context context, cl_device_id device_id, const char* file_name, const char* kernel_name) {
    cl_program program = create_cl_program(context, file_name);
    clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, kernel_name, NULL);
    clReleaseProgram(program);
    printf("made %s\n", kernel_name);
    return kernel;
}

#endif /* common_h */
