#include "myApp.h"
#include "JournalUtils.h"

#include <iostream>
#include <fstream> 
#include <filesystem>  
#include <inotify-cpp/NotifierBuilder.h> 

myApp::myApp()
    : mWatchedDirectories(), mNotifiers(), showDirectoryChooser(false), showDirError(false)
{
    mInputDirectory[0] = '\0';
}

myApp::~myApp()
{
    // end inotify watches
    for (auto& thread : mNotifierThreads) 
    {
        if (thread.joinable()) 
        {
            thread.join();
        }
    }
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
    static ImVec2 window1_size = ImVec2(parent_display_size.x / 4, parent_display_size.y);
    static ImVec2 window2_size = ImVec2(parent_display_size.x / 2, parent_display_size.y);
    static ImVec2 window2_pos = ImVec2(window1_size.x, 0);

    // Toolbar ===========================
    ImGui::SetNextWindowSize(window1_size);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, parent_display_size.y), ImVec2(parent_display_size.x - 100, parent_display_size.y));

    ImGui::Begin("toolbar", nullptr, window_flags);
    window1_size = ImGui::GetWindowSize();
    SimpleDirectoryChooser(); 
    ShowJournalInit(); 
    Spacer(); 
    ShowFileChooser(); 
    ImGui::End();

    // Update window2 position and size based on window1
    window2_pos = ImVec2(window1_size.x, 0);
    window2_size = ImVec2(parent_display_size.x - window1_size.x, parent_display_size.y);

    // File View =========================
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


    mEntries.clear();
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(mCurrentDirPath)) 
        {
            if (!entry.path().filename().string().empty() && entry.is_directory()) 
            {
                mEntries.push_back(entry);
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
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { mCurrentDirPath = mCurrentDirPath.parent_path(); }
        ImGui::Text("Current Path: %s", mCurrentDirPath.string().c_str());

        if (ImGui::BeginListBox(" ")) 
        {
            for (const auto& entry : mEntries) 
            {
                std::string filename = entry.path().filename().string();
                if (!filename.empty() && ImGui::Selectable(filename.c_str())) 
                {
                    if (std::filesystem::is_directory(entry)) 
                    {
                        std::cout << entry.path(); 
                        mCurrentDirPath = entry.path();
                        mEntries.clear();
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
    ImGui::SeparatorText("Select a Directory"); 
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
            mCurrentDirPath = enteredPath;
            showDirError = false; 

            //get file paths in directory
            mFilesInDir.clear();
            try
            {
                for (const auto& file : std::filesystem::directory_iterator(mCurrentDirPath)) 
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

    ImGui::Spacing(); 
    std::string currentDir = "Current directory: " + mCurrentDirPath.string(); 
    
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
    Spacer(); 
    ImGui::SeparatorText("Initilize Journal for Current Dir"); 
    ImGui::PushItemWidth(400); 
    if (ImGui::Button("Init journaling"))
    {
        journalUtils::initilizeDirectory(mCurrentDirPath); 
        if(mWatchedDirectories.find(mCurrentDirPath) == mWatchedDirectories.end())
        {
            addInotifyWatch(); 
        }
        else
        {
            std::cout << "Directory already being watched by Inotify" << std::endl; 
        }
    }

    ImGui::PopItemWidth(); 

} 

void myApp::ShowFileChooser()
{
    ImGui::PushItemWidth(150); 

    ImGui::SeparatorText("Select a File"); 
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

void myApp::addInotifyWatch()
{
    std::cout << "current directory: " << mCurrentDirPath << std::endl; 
     auto handleNotification = [&](inotify::Notification notification)
    {
        std::cout << notification.event << std::endl; 
        if (notification.event == inotify::Event::create)
        {
            if (notification.path.filename().string().find("_journal") == std::string::npos) //*better identifier for journal files 
            {
                std::cout << '\n' << "----------------" << std::endl; 
                std::cout << "Event Type: " << notification.event << std::endl;   
                journalUtils::createNewJournal(notification.path); 
                std::cout << "----------------" << std::endl; 
            }
        }
        else if (notification.event == inotify::Event::close_write)
        {
            if (notification.path.filename().string().find("_journal") == std::string::npos)
            {
                std::cout << '\n' << "----------------" << std::endl; 
                std::cout << "Event Type: " << notification.event << std::endl;   
                journalUtils::updateJournal(notification.path); 
                std::cout << "----------------" << std::endl; 
            }
        }      
    };

    //events to watch for 
    auto events = {
            inotify::Event::access,
            inotify::Event::create,
            inotify::Event::modify,
            inotify::Event::remove,
            inotify::Event::close_write, 
        };

    //build notifier for handling events 
    auto notifier = inotify::BuildNotifier()
        .watchPathRecursively(mCurrentDirPath)
        .ignoreFileOnce("fileIgnoredOnce")
        .ignoreFile("FileIgnored")
        .onEvents(events, handleNotification);

    auto notifier_ptr = std::make_shared<decltype(notifier)>(std::move(notifier));
    {
        std::lock_guard<std::mutex> lock(mNotifiersMutex);
        mNotifiers.push_back(notifier_ptr);
    }

    std::thread notifierThread([notifier_ptr]() 
    {
        notifier_ptr->run();
    });

    mNotifierThreads.emplace_back(std::move(notifierThread));
    mWatchedDirectories.insert(mCurrentDirPath);
} 

void myApp::stopNotifierThreads() 
{
    std::lock_guard<std::mutex> lock(mNotifiersMutex);
    for (auto& notifier_ptr : mNotifiers)
    {
        notifier_ptr->stop(); 
    }

    for (auto& thread : mNotifierThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
