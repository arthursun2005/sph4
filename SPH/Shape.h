//
//  Shape.h
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Shape_h
#define Shape_h

#include "common.h"

/// axis aligned bounding box
struct AABB
{
    vec2 lowerBound;
    vec2 upperBound;
    
    AABB() {}
    
    AABB(const vec2& lowerBound, const vec2& upperBound) : lowerBound(lowerBound), upperBound(upperBound) {}
    
    inline float area() const {
        return (upperBound.x - lowerBound.x) * (upperBound.y - lowerBound.y);
    }
    
    inline float perimeter() const {
        return 2.0f * (upperBound.x - lowerBound.x + upperBound.y - lowerBound.y);
    }
};

inline vec2 min(const vec2& a, const vec2& b) {
    return vec2(std::min(a.x, b.x), std::min(a.y, b.y));
}

inline vec2 max(const vec2& a, const vec2& b) {
    return vec2(std::max(a.x, b.x), std::max(a.y, b.y));
}

inline AABB combine_aabb(const AABB& a, const AABB& b) {
    return AABB(min(a.lowerBound, b.lowerBound), max(a.upperBound, b.upperBound));
}

inline bool touches(const AABB& a, const AABB& b) {
    vec2 d1 = a.upperBound - b.lowerBound;
    vec2 d2 = b.upperBound - a.lowerBound;
    return d1.x > 0.0f && d1.y > 0.0f && d2.x > 0.0f && d2.y > 0.0f;
}

struct Shape
{
    vec2* vertices;
    vec2* normals;
    int count;
    
    Shape() : count(0) {}
    
    Shape(std::initializer_list<float>& il) {
        initialize(il);
    }
    
    ~Shape() {
        destory();
    }
    
    void destory() {
        if(count != 0) {
            Free(vertices);
            Free(normals);
            count = 0;
        }
    }
    
    void initialize(int n) {
        if(count != n) {
            destory();
            count = n;
            vertices = (vec2*)Alloc(count * sizeof(vec2));
            normals = (vec2*)Alloc(count * sizeof(vec2));
        }
    }
    
    void initialize(const std::initializer_list<float>& il) {
        int k2 = (int)il.size();
        
        if((k2&1) != 0)
            throw std::invalid_argument("An EVEN number of floats makes up a list of vertices");
        
        initialize((vec2*)il.begin(), (vec2*)il.end());
    }
    
    void initialize(const vec2* begin, const vec2* end) {
        initialize((int)(end - begin));
        memcpy(vertices, begin, sizeof(vec2) * count);
        
        for(int i = 0; i < count; ++i) {
            int i2 = i != count - 1 ? i + 1 : 0;
            normals[i] = (vertices[i2] - vertices[i]).I().norm();
        }
    }
    
    inline void initializeAsBox(const vec2& p, float hw, float hh) {
        initialize({
            p.x - hw, p.y - hh,
            p.x + hw, p.y - hh,
            p.x + hw, p.y + hh,
            p.x - hw, p.y + hh
        });
    }
    
    void initializeAsCircle(const vec2& p, float r, int n) {
        initialize(n);
        
        float a;
        for(int i = 0; i < count; ++i) {
            a = 2.0f * M_PI * i / (float) count;
            vertices[i].x = r * cosf(a) + p.x;
            vertices[i].y = r * sinf(a) + p.y;
        }
        
        for(int i = 0; i < count; ++i) {
            int i2 = i != count - 1 ? i + 1 : 0;
            normals[i] = (vertices[i2] - vertices[i]).I().norm();
        }
    }
    
    inline bool includes(const vec2& p) const {
        for(int i = 0; i < count; ++i)
            if(dot(normals[i], p - vertices[i]) > 0.0f)
                return false;
        return true;
    }
    
    inline AABB aabb() const {
        AABB q(vertices[0], vertices[0]);
        for(int i = 1; i < count; ++i) {
            q.lowerBound = min(vertices[i], q.lowerBound);
            q.upperBound = max(vertices[i], q.upperBound);
        }
        return q;
    }
};

#endif /* Shape_h */
