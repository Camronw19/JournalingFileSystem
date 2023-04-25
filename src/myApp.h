#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h> 
#include <imgui_impl_opengl3.h>
#include <inotify-cpp/NotifierBuilder.h>

#include <vector>
#include <string> 
#include <filesystem>
#include <thread> 
#include <set>
#include <atomic>
#include <mutex> 

class myApp {
public: 

    myApp(); 
    ~myApp(); 
    void Init(GLFWwindow*, const char*); 
    virtual void ShowWindow(); 
    void Render(); 
    void Shutdown(); 
    void NewFrame(); 
    void stopNotifierThreads(); 

private: 
    bool showDirectoryChooser; 
    bool showDirError; 

    std::vector<std::thread> mNotifierThreads; 
    std::vector<std::shared_ptr<decltype(inotify::BuildNotifier())>> mNotifiers;
    std::mutex mNotifiersMutex;
    std::set<std::filesystem::path> mWatchedDirectories; 

    std::filesystem::path mCurrentDirPath; 
    std::vector<std::filesystem::directory_entry> mEntries;
    std::vector<std::filesystem::path> mFilesInDir; 
    std::vector<std::string> mFileContents;
    char mInputDirectory[128]; 

    // helper methods
    std::vector<std::string> getFileContents(std::filesystem::path);

    // components 
    void ShowMenu(); 
    void ShowFiles(); 
    void ShowFileContent(); 
    void DirectoryChooser(); 
    void SimpleDirectoryChooser(); 
    void Spacer(); 
    void ShowJournalInit(); 
    void ShowFileChooser();

    // inotify  
    void addInotifyWatch(); 
};