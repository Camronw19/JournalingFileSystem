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
    bool mShowDirError; 

    // inotify 
    std::vector<std::thread> mNotifierThreads; 
    std::vector<std::shared_ptr<decltype(inotify::BuildNotifier())>> mNotifiers;
    std::mutex mNotifiersMutex;
    std::set<std::filesystem::path> mWatchedDirectories; 

    // Current Directory 
    std::filesystem::path mCurrentDirPath; 
    std::vector<std::filesystem::path> mFilesInDir; 
    std::vector<std::filesystem::directory_entry> mEntries;

    // Current File
    std::filesystem::path mCurrentFile; 
    std::vector<std::string> mFileContents;
    std::vector<std::string> mJournalContents;
    std::map<std::string, std::string> mReconstructionDates;
    std::string mReconstructionLineNum;  
    char mInputDirectory[128]; 


    // helper methods
    std::vector<std::string> getFileContents(std::filesystem::path);
    void setFileContents(); 
    void getFilesInDirectory(); 
    bool isRegularFile(const std::filesystem::directory_entry&); 

    // components 
    void ShowDirectoryChooser(); 
    void ShowJournalInit(); 
    void ShowFileChooser();
    void ShowJournalReconstruction(); 
    void ShowFiles(); 
    void ShowFileContent(const std::vector<std::string>&); 
    void Spacer(); 

    // inotify  
    void addInotifyWatch(); 
};