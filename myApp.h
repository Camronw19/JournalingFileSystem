#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h> 
#include <imgui_impl_opengl3.h>

class myApp {
public: 
    void Init(GLFWwindow*, const char*); 
    void NewFrame(); 
    virtual void Update(); 
    void Render(); 
    void Shutdown(); 
private: 
};