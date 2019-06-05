//
//  Grpahics.h
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Grpahics_h
#define Grpahics_h

#include "utils.h"
#include "ParticleSystem.hpp"

class PSGraphic
{
    
private:
    
    int bufferSize;
    
    GLuint positions;
    GLuint vao;
    
    const ParticleSystem* ps;
    
    glProgram renderer;
    
public:
    
    PSGraphic(ParticleSystem* ps) : ps(ps) {}
    
    void initialize();
    
    void destory();
    
    void load();
    
    void draw(GLuint target, const Frame& frame);
};

void PSGraphic::initialize() {
    renderer.initialize_with_header("point.vs", "fill.fs", "common.glsl");
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &positions);
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    
    bufferSize = 0;
}

void PSGraphic::destory() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &positions);
    renderer.destory();
}

void PSGraphic::load() {
    if(ps->capacity > bufferSize) {
        bufferSize = ps->capacity;
        
        glBindBuffer(GL_ARRAY_BUFFER, positions);
        glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(vec2), 0, GL_STREAM_DRAW);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glBufferSubData(GL_ARRAY_BUFFER, 0, ps->count * sizeof(vec2), ps->positions);
}

void PSGraphic::draw(GLuint target, const Frame& frame) {
    load();
    
    renderer.bind();
    renderer.uniform2f("scl", frame.scl/(float)frame.w, frame.scl/(float)frame.h);
    renderer.uniform2f("offset", frame.offset.x, frame.offset.y);
    renderer.uniform1f("size", 0.25f * ps->diameter * frame.scl);
    renderer.uniform4f("fill", 0.0f, 0.0f, 1.0f, 1.0f);
    
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, target);
    glBindVertexArray(vao);
    glViewport(frame.x, frame.y, frame.w, frame.h);
    glDrawArrays(GL_POINTS, 0, ps->count);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}


#endif /* Grpahics_h */
