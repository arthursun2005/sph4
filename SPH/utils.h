//
//  utils.h
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unistd.h>
#include <string>

#include "common.h"

enum shader_types
{
    gl_geometry_shader = GL_GEOMETRY_SHADER,
    gl_fragment_shader = GL_FRAGMENT_SHADER,
    gl_vertex_shader = GL_VERTEX_SHADER,
    gl_compute_shader = GL_COMPUTE_SHADER,
    gl_header_shader = 0
};

inline bool end_in(const std::string& str, const std::string& end) {
    size_t s1 = end.size();
    size_t s2 = str.size();
    
    if(s1 > s2)
        return false;
    
    for(size_t i = 0; i < s1; ++i)
        if(end[s1 - i - 1] != str[s2 - i - 1]) return false;
    
    return true;
}

inline bool start_with(const std::string& str, const std::string& start) {
    size_t s1 = start.size();
    size_t s2 = str.size();
    
    if(s1 > s2)
        return false;
    
    for(size_t i = 0; i < s1; ++i)
        if(start[i] != str[i]) return false;
    
    return true;
}

struct glShader
{
    std::string str;
    shader_types type;
    
    std::string file;
    
    glShader(const char* file_name) {
        fstr(file_name, &str);
        
        if(end_in(file_name, ".cs")) {
            type = gl_compute_shader;
        }else if(end_in(file_name, ".fs")) {
            type = gl_fragment_shader;
        }else if(end_in(file_name, ".vs")) {
            type = gl_vertex_shader;
        }else if(end_in(file_name, ".gs")) {
            type = gl_geometry_shader;
        }else if(end_in(file_name, ".glsl")) {
            type = gl_header_shader;
        }else{
            throw std::invalid_argument("file name has to end in [.cs], [.fs], [.gs], [.vs] or [.glsl]");
        }
        
        file.assign(file_name);
    }
    
    glShader(const char* file_name, shader_types type) : type(type) {
        fstr(file_name, &str);
        file.assign(file_name);
    }
    
    inline void apply_header(const glShader& shader) {
        assert(shader.type == gl_header_shader);
        str = shader.str + str;
    }
};

inline void apply_headers(const std::initializer_list<glShader*>& shaders, const glShader& header) {
    for(glShader* shader : shaders)
        shader->apply_header(header);
}

inline GLuint compileShader(const glShader& _shader) {
    GLuint shader = glCreateShader(_shader.type);
    const char* c_str = _shader.str.c_str();
    glShaderSource(shader, 1, &c_str, NULL);
    
    glCompileShader(shader);
    
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    
    if(status == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        GLchar log[length];
        glGetShaderInfoLog(shader, length, NULL, log);
        printf("%s \n", log);
    }
    
    printf("compiled %s\n", _shader.file.c_str());
    
    return shader;
}

inline GLuint compileProgram(const std::initializer_list<GLuint>& shaders) {
    GLuint program = glCreateProgram();
    
    for(GLuint shader : shaders)
        glAttachShader(program, shader);
    
    glLinkProgram(program);
    
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        GLchar log[length];
        glGetProgramInfoLog(program, length, NULL, log);
        printf("%s \n", log);
    }
    
    return program;
}

inline void deleteShaders(GLuint program, const std::initializer_list<GLuint>& shaders) {
    for(GLuint shader : shaders) {
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }
}

struct glProgram
{
    GLuint program;
    
    void initialize(const glShader& shader) {
        assert(shader.type == gl_compute_shader);
        GLuint cs = compileShader(shader);
        program = compileProgram({cs});
        deleteShaders(program, {cs});
    }
    
    void initialize(const glShader& shader1, const glShader& shader2) {
        assert(shader1.type == gl_vertex_shader);
        assert(shader2.type == gl_fragment_shader);
        
        GLuint vs = compileShader(shader1);
        GLuint fs = compileShader(shader2);
        
        program = compileProgram({vs, fs});
        deleteShaders(program, {vs, fs});
    }
    
    void initialize(const glShader& shader1, const glShader& shader2, const glShader& shader3) {
        assert(shader1.type == gl_vertex_shader);
        assert(shader2.type == gl_geometry_shader);
        assert(shader3.type == gl_fragment_shader);
        
        GLuint vs = compileShader(shader1);
        GLuint gs = compileShader(shader2);
        GLuint fs = compileShader(shader3);
        
        program = compileProgram({vs, gs, fs});
        deleteShaders(program, {vs, gs, fs});
    }
    
    void initialize_with_header(const glShader& shader1, const glShader& shader2) {
        glShader s1 = shader1;
        s1.apply_header(shader2);
        initialize(s1);
    }
    
    void initialize_with_header(const glShader& shader1, const glShader& shader2, const glShader& shader3) {
        glShader s1 = shader1;
        glShader s2 = shader2;
        apply_headers({&s1, &s2}, shader3);
        initialize(s1, s2);
    }
    
    void initialize_with_header(const glShader& shader1, const glShader& shader2, const glShader& shader3, const glShader& shader4) {
        glShader s1 = shader1;
        glShader s2 = shader2;
        glShader s3 = shader3;
        apply_headers({&s1, &s2, &s3}, shader4);
        initialize(s1, s2, s3);
    }
    
    void destory() {
        glDeleteProgram(program);
    }
    
    inline void bind() {
        glUseProgram(program);
    }
    
    inline void uniform1i(const char* n, int i0) const {
        glUniform1i(glGetUniformLocation(program, n), i0);
    }
    
    inline void uniform2i(const char* n, int i0, int i1) const {
        glUniform2i(glGetUniformLocation(program, n), i0, i1);
    }
    
    inline void uniform3i(const char* n, int i0, int i1, int i2) const {
        glUniform3i(glGetUniformLocation(program, n), i0, i1, i2);
    }
    
    inline void uniform4i(const char* n, int i0, int i1, int i2, int i3) const {
        glUniform4i(glGetUniformLocation(program, n), i0, i1, i2, i3);
    }
    
    inline void uniform1ui(const char* n, unsigned int i0) const {
        glUniform1ui(glGetUniformLocation(program, n), i0);
    }
    
    inline void uniform2ui(const char* n, unsigned int i0, unsigned int i1) const {
        glUniform2ui(glGetUniformLocation(program, n), i0, i1);
    }
    
    inline void uniform3ui(const char* n, unsigned int i0, unsigned int i1, unsigned int i2) const {
        glUniform3ui(glGetUniformLocation(program, n), i0, i1, i2);
    }
    
    inline void uniform4ui(const char* n, unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3) const {
        glUniform4ui(glGetUniformLocation(program, n), i0, i1, i2, i3);
    }
    
    inline void uniform1f(const char* n, float f0) const {
        glUniform1f(glGetUniformLocation(program, n), f0);
    }
    
    inline void uniform2f(const char* n, float f0, float f1) const {
        glUniform2f(glGetUniformLocation(program, n), f0, f1);
    }
    
    inline void uniform3f(const char* n, float f0, float f1, float f2) const {
        glUniform3f(glGetUniformLocation(program, n), f0, f1, f2);
    }
    
    inline void uniform4f(const char* n, float f0, float f1, float f2, float f3) const {
        glUniform4f(glGetUniformLocation(program, n), f0, f1, f2, f3);
    }
};

inline void destoryPrograms(const std::initializer_list<glProgram*>& programs) {
    for(glProgram* p : programs)
        p->destory();
}

#endif /* utils_h */
