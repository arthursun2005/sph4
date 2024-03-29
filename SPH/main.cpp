//
//  main.cpp
//  SPH
//
//  Created by Arthur Sun on 6/4/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include <iostream>
#include "Grpahics.h"
#include "ParticleSystem.hpp"

GLFWwindow *window;

double mouseX, mouseY;
double pmouseX = mouseX, pmouseY = mouseY;
int width = 1280;
int height = 840;
float dt = 0.016f;
float D = 0.05f;

Frame frame;

float dMouseX = 0.0f;
float dMouseY = 0.0f;

vec2 gravity(0.0f, -9.8f);

float timeBtwFrames = 0.016f;

#ifdef DEBUG

int framesPerSecond = 0;
float lastSecondTime = glfwGetTime();

#endif

#define mouse_scale 0.95f

#define scroll_scale 0.001f

ParticleSystem ps(gravity);
PSGraphic renderer(&ps);

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
}

inline vec2 getMouse() {
    float k = 2.0f / frame.scl;
    vec2 mouse((mouseX * 2.0f - width) * k, (mouseY * 2.0f - height) * -k);
    mouse -= frame.offset;
    return mouse;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_RELEASE) {
        vec2 mouse = getMouse();
        
        if(key == GLFW_KEY_A) {
            Shape shape;
            shape.initializeAsCircle(mouse, 1.0f, 60);
            
            ps.add(shape, vec2(0.0f, 0.0f));
        }
        
        if(key == GLFW_KEY_S) {
            Shape shape;
            shape.initializeAsCircle(mouse, 0.2f, 60);
            
            ps.add(shape, vec2(100.0f, 0.0f));
        }
        
        if(key == GLFW_KEY_B) {
            Shape shape;
            shape.initializeAsBox(mouse, 1.0f, 10.0f);
            
            ps.add(shape, vec2(0.0f, 0.0f));
        }
        
        if(key == GLFW_KEY_Q) {
            Shape shape;
            shape.initializeAsBox(mouse, 1.0f, 1.0f);
            
            ps.add(shape, vec2(0.0f, 0.0f));
        }
        
        if(key == GLFW_KEY_R) {
            ps.clear();
        }
        
        if(key == GLFW_KEY_N) {
            printf("%d\n", ps.getCount());
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    frame.scl *= exp(yoffset * scroll_scale);
}

void resizeCallback(GLFWwindow* window, int w, int h)
{
    width = w;
    height = h;
    frame.w = width * 2;
    frame.h = height * 2;
}

int main(int argc, const char * argv[]) {
    srand((unsigned int)time(0));
    
    if(!glfwInit())
        return EXIT_FAILURE;
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    
    window = glfwCreateWindow(width, height, "SPH", NULL, NULL);
    
    glfwMakeContextCurrent(window);
    
    glewInit();
    
    const unsigned char* version = glGetString(GL_VERSION);
    
    printf("%s\n", version);
    
    glfwSetMouseButtonCallback(window, mouseCallback);
    
    GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    
    glfwSetCursor(window, cursor);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    
    mouseX = width * 0.5f;
    mouseY = height * 0.5f;
    
    renderer.initialize();
    ps.initialize(D);
    
    frame.x = 0;
    frame.y = 0;
    frame.w = width * 2;
    frame.h = height * 2;
    frame.scl = 200.0f;
    frame.offset = vec2(0.0f, 0.0f);
    
    do {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwGetCursorPos(window, &mouseX, &mouseY);
        
        float currentTime = glfwGetTime();
        bool press = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        
#ifdef DEBUG
        ++framesPerSecond;
        if(currentTime - lastSecondTime >= 1.0f) {
            printf("%f ms/frame \n", 1000.0f * (currentTime - lastSecondTime)/(float)framesPerSecond);
            framesPerSecond = 0;
            lastSecondTime = currentTime;
        }
#endif
        
        if(press) {
            dMouseX = (mouseX - pmouseX);
            dMouseY = (pmouseY - mouseY);
        }else{
            dMouseX *= mouse_scale;
            dMouseY *= mouse_scale;
        }
        
        frame.offset.x += dMouseX * 4.0f / frame.scl;
        frame.offset.y += dMouseY * 4.0f / frame.scl;
        
        ps.step(dt, 6);
        
        renderer.draw(0, frame);
        
        glfwPollEvents();
        glfwSwapBuffers(window);
        
        pmouseX = mouseX;
        pmouseY = mouseY;

        float finish = glfwGetTime();
        
        float ssecs = std::max(timeBtwFrames - (finish - currentTime), 0.0f);

        usleep(useconds_t(ssecs * 1000000.0f));
    } while (glfwWindowShouldClose(window) == GL_FALSE && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
    renderer.destory();
    glfwDestroyCursor(cursor);
    glfwTerminate();
    return EXIT_SUCCESS;
}

