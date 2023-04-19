#include "myApp.h"

#include <iostream>
#include <fstream> 
#include <filesystem>  

myApp::myApp()
{
    current_path = std::filesystem::current_path();
    showDirectoryChooser = false; 
}

void myApp::Init(GLFWwindow* window, const char* glsl_version)
{
    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext(); 
    ImGuiIO &io = ImGui::GetIO(); 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version); 
    ImGui::StyleColorsDark(); 
}

void myApp::ShowWindow() 
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize = io.DisplaySize;
    ImVec2 windowPos = ImVec2(0, 0);

    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowContentSize(windowSize);

    ImGuiWindowFlags window_flags = 0; 
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::Begin("Journal File System", nullptr, window_flags);
    
    ShowMenu(); 
    DirectoryChooser(); 
    ShowFiles(); 

    ImGui::End();
}

void myApp::Render()
{
    ImGui::Render(); 
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 
}

void myApp::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void myApp::NewFrame()
{
    // feed inputs to dear imgui, start new frame 
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

// components 

void myApp::ShowMenu()
{
     if (ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("Menu"))
        {
            if(ImGui::MenuItem("Open Directory..."))
            {
                std::cout << "Opening directory" << std::endl; 
                showDirectoryChooser = true; 
            }
            if(ImGui::MenuItem("Select File..."))
            {
                std::cout << "Opening File" << std::endl; 
                std::cout << "Reading File" << std::endl; 
                //get file path
                
                //=============
                std::filesystem::path filePath("/home/cam/Documents/Projects/glfwTest/testFile.txt");
                mFileContents = getFileContents(filePath); 
            }
            ImGui::EndMenu(); 
        }
        if(ImGui::BeginMenu("Tools"))
        {
            if(ImGui::MenuItem("Reconstruct File"))
            {
                std::cout << "Reconstructing file" << std::endl; 
            }
            ImGui::EndMenu(); 
        }
        ImGui::EndMenuBar(); 
    } 
}

void myApp::ShowFiles()
{
    ImGuiWindowFlags window_flags = 0;  
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImVec2 parentWindowSize = ImGui::GetWindowSize();
    ImVec2 childWindowSize = ImVec2(parentWindowSize.x * 0.5f - 20, parentWindowSize.y);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

    // file contents 
    ImGui::BeginChild("File", childWindowSize, true, window_flags); 
    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("File contents");
        ImGui::EndMenuBar();   
    }

    ShowFileContent(); 
    
    ImGui::EndChild();

    ImGui::SameLine(); 

    // journal contents 
    ImGui::BeginChild("Journal", childWindowSize, true, window_flags); 
    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("Journal contents");
        ImGui::EndMenuBar();   
    }
    ImGui::EndChild();

    ImGui::PopStyleVar();
} 

std::vector<std::string> myApp::getFileContents(std::filesystem::path path)
{
    std::vector<std::string> fileContents; 

    std::filesystem::path filePath(path);
    std::ifstream file(filePath); 

    if(!file.is_open())
    {
        std::cout << "Could not open file" << std::endl; 
    }
    else
    {
        fileContents.reserve(20); 
        std::string line;

        while(std::getline(file, line))
        {
            fileContents.push_back(line); 
        }
    }

    return fileContents; 
} 

void myApp::ShowFileContent()
{
    if (!mFileContents.empty())
    {
        for (const auto& line : mFileContents)
        {
            ImGui::TextWrapped("%s", line.c_str()); 
        }
    }
} 

void myApp::DirectoryChooser() {


    entries.clear();
    for (const auto& entry : std::filesystem::directory_iterator(current_path)) 
    {
        if (!entry.path().filename().string().empty()) 
        {
            entries.push_back(entry);
        }
    }

    if (showDirectoryChooser) {
    ImGui::Begin("Directory Chooser", &showDirectoryChooser);
    ImGui::Text("Current Path: %s", current_path.string().c_str());

    if (ImGui::BeginListBox("")) {
        for (const auto& entry : entries) {
            std::string filename = entry.path().filename().string();
            if (!filename.empty() && ImGui::Selectable(filename.c_str())) {
                if (std::filesystem::is_directory(entry)) {
                    current_path = entry;
                    entries.clear();
                    for (const auto& new_entry : std::filesystem::directory_iterator(current_path)) {
                        if (!new_entry.path().filename().string().empty()) {
                            entries.push_back(new_entry);
                        }
                    }
                } else {
                    // Handle file selection, if necessary
                }
            }
        }
        ImGui::EndListBox();
    }

    ImGui::End();
}
}