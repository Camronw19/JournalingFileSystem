#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h> 
#include <imgui_impl_opengl3.h>

#include <vector>
#include <string> 
#include <filesystem>

class myApp {
public: 

    myApp(); 
    void Init(GLFWwindow*, const char*); 
    virtual void ShowWindow(); 
    void Render(); 
    void Shutdown(); 
    void NewFrame(); 

private: 
    bool showDirectoryChooser; 

    std::filesystem::path current_path;
    std::vector<std::filesystem::directory_entry> entries;
    std::vector<std::string> mFileContents;

    // helper methods
    std::vector<std::string> getFileContents(std::filesystem::path);

    // components 
    void ShowMenu(); 
    void ShowFiles(); 
    void ShowFileContent(); 
    void DirectoryChooser(); 

};