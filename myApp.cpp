#include "myApp.h"

#include <iostream>
#include <fstream> 
#include <filesystem>  

myApp::myApp()
{
    showDirectoryChooser = false; 
    showDirError = false; 
    mInputDirectory[0] = {'\0'};
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

   // Set window flags 
    ImGuiWindowFlags window_flags = 0; 
    window_flags |= ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_HorizontalScrollbar;

    ImVec2 parent_display_size = ImGui::GetIO().DisplaySize;

    // Sizing 
    static ImVec2 window1_size = ImVec2(parent_display_size.x / 2, parent_display_size.y);
    static ImVec2 window2_size = ImVec2(parent_display_size.x / 2, parent_display_size.y);
    static ImVec2 window2_pos = ImVec2(window1_size.x, 0);

    // Toolbar
    ImGui::SetNextWindowSize(window1_size);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, parent_display_size.y), ImVec2(parent_display_size.x - 100, parent_display_size.y));

    ImGui::Begin("toolbar", nullptr, window_flags);
    window1_size = ImGui::GetWindowSize();
    ShowMenu(); 
    SimpleDirectoryChooser(); 
    ShowJournalInit(); 
    Spacer(); 
    ShowFileChooser(); 
    ImGui::End();

    // Update window2 position and size based on window1
    window2_pos = ImVec2(window1_size.x, 0);
    window2_size = ImVec2(parent_display_size.x - window1_size.x, parent_display_size.y);

    // File view 
    ImGui::SetNextWindowSize(window2_size);
    ImGui::SetNextWindowPos(window2_pos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, parent_display_size.y), ImVec2(parent_display_size.x - 100, parent_display_size.y));

    ImGui::Begin("file view", nullptr, window_flags);
    window2_size = ImGui::GetWindowSize();
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
    window_flags |= ImGuiWindowFlags_HorizontalScrollbar;

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
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(mCurrentPath)) 
        {
            if (!entry.path().filename().string().empty() && entry.is_directory()) 
            {
                entries.push_back(entry);
            }
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cout << "Error accesing directory" << std::endl; 
    }
    

    if (showDirectoryChooser) 
    {
        ImGui::Begin("Directory Chooser", &showDirectoryChooser);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { mCurrentPath = mCurrentPath.parent_path(); }
        ImGui::Text("Current Path: %s", mCurrentPath.string().c_str());

        if (ImGui::BeginListBox(" ")) 
        {
            for (const auto& entry : entries) 
            {
                std::string filename = entry.path().filename().string();
                if (!filename.empty() && ImGui::Selectable(filename.c_str())) 
                {
                    if (std::filesystem::is_directory(entry)) 
                    {
                        std::cout << entry.path(); 
                        mCurrentPath = entry.path();
                        entries.clear();
                    } else 
                    {
                        // Handle file selection, if necessary
                    }
                }
            }

        ImGui::EndListBox();
        }
        
    ImGui::End();
    }
}

void myApp::SimpleDirectoryChooser()
{
    ImGui::PushItemWidth(200); 
    ImGui::InputTextWithHint(" ", "Enter directory path", mInputDirectory, IM_ARRAYSIZE(mInputDirectory)); 
    ImGui::PopItemWidth(); 


    ImGui::SameLine(); 

    if (ImGui::Button("Submit"))
    {
        std::cout << mInputDirectory << std::endl; 
        std::filesystem::path enteredPath(mInputDirectory);
        memset(mInputDirectory, '\0', 128 ); 

        if (std::filesystem::exists(enteredPath) && std::filesystem::is_directory(enteredPath))
        {
            mCurrentPath = enteredPath;
            showDirError = false; 

            //get file paths in directory
            mFilesInDir.clear();
            try
            {
                for (const auto& file : std::filesystem::directory_iterator(mCurrentPath)) 
                {
                    if (!file.path().filename().string().empty() && file.is_regular_file()) 
                    {
                        mFilesInDir.push_back(file);
                    }
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                std::cout << "Error accesing directory" << std::endl; 
            }
    
        }
        else
        {
            std::cerr << "Error: The entered path is not a valid directory." << std::endl;
            showDirError = true; 
        }
    }

    std::string currentDir = "Current directory: " + mCurrentPath.string(); 

    if (showDirError) { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),  "The entered path is not a valid directory"); }
    else { ImGui::Text("%s",currentDir.c_str()); }
}

void myApp::Spacer()
{
    float custom_vertical_spacing = 20.0f;
    ImGui::Dummy(ImVec2(0.0f, custom_vertical_spacing));
}

void myApp::ShowJournalInit()
{
    ImGui::PushItemWidth(400); 
    if (ImGui::Button("Add journal to directory"))
    {

    }

    ImGui::PopItemWidth(); 

} 

void myApp::ShowFileChooser()
{
    ImGui::PushItemWidth(150); 

    ImGui::SeparatorText("Select a file"); 
    if (ImGui::BeginListBox(" ")) 
    {
        for (const auto& file : mFilesInDir) 
        {
            std::string filename = file.filename().string();
            if (!filename.empty() && ImGui::Selectable(filename.c_str())) 
            {
                mFileContents = getFileContents(file); 
            }
        }

    ImGui::EndListBox();
    }

    ImGui::PopItemWidth(); 
}