#include "myApp.h"
#include "JournalUtils.h"

#include <iostream>
#include <fstream> 
#include <filesystem>  
#include <inotify-cpp/NotifierBuilder.h> 

myApp::myApp()
    : mWatchedDirectories(), mNotifiers(), mShowDirError(false), mInputDirectory {}
{

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


    // Sizing 
    ImVec2 parent_display_size = ImGui::GetIO().DisplaySize;
    static ImVec2 window1_size = ImVec2(parent_display_size.x / 4, parent_display_size.y);
    static ImVec2 window2_size = ImVec2(parent_display_size.x / 2, parent_display_size.y);
    static ImVec2 window2_pos = ImVec2(window1_size.x, 0);

    // Toolbar 
    ImGui::SetNextWindowSize(window1_size);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, parent_display_size.y), ImVec2(parent_display_size.x - 100, parent_display_size.y));

    ImGui::Begin("toolbar", nullptr, window_flags);
    window1_size = ImGui::GetWindowSize();
    ShowDirectoryChooser(); 
    ShowJournalInit(); 
    Spacer(); 
    ShowFileChooser(); 
    Spacer(); 
    ShowJournalReconstruction(); 
    ImGui::End();

    // Update window2 position and size based on window1
    window2_pos = ImVec2(window1_size.x, 0);
    window2_size = ImVec2(parent_display_size.x - window1_size.x, parent_display_size.y);

    // File View
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
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

// components 
void myApp::ShowDirectoryChooser()
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
            mShowDirError = false; 
            getFilesInDirectory(); 
        }
        else
        {
            mShowDirError = true; 
        }
    }

    ImGui::Spacing(); 
    std::string currentDir = "Current directory: " + mCurrentDirPath.string(); 
    
    if (mShowDirError) { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),  "The entered path is not a valid directory"); }
    else { ImGui::Text("%s",currentDir.c_str()); }
}

void myApp::ShowFiles()
{
    // window flags
    ImGuiWindowFlags window_flags = 0;  
    window_flags |= ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_HorizontalScrollbar;

    // sizing 
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
    ShowFileContent(mFileContents); 
    ImGui::EndChild();

    ImGui::SameLine(); 

    // journal contents 
    ImGui::BeginChild("Journal", childWindowSize, true, window_flags); 
    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("Journal contents");
        ImGui::EndMenuBar();   
    }
    ShowFileContent(mJournalContents); 
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
        std::cerr << "Could not open file" << std::endl; 
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

void myApp::ShowFileContent(const std::vector<std::string>& file)
{
    if (!file.empty())
    {
        for (const auto& line : file)
        {
            ImGui::TextWrapped("%s", line.c_str()); 
        }
    }
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
           try
            {
                addInotifyWatch(); 
            }
            catch (const std::exception& e)
            {
                std::cout << "Exception occurred while adding Inotify watch: " << e.what() << std::endl;
                // Handle the exception or display an error message as needed.
            }
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
                mCurrentFile = file; 
                setFileContents(); 
                mReconstructionDates = journalUtils::getReconstructionDates(mCurrentFile); 
            }
        }
        ImGui::EndListBox();
    }
    ImGui::PopItemWidth(); 
}

void myApp::addInotifyWatch()
{
     auto handleNotification = [&](inotify::Notification notification)
    {
        if (notification.event == inotify::Event::create)
        {
            if (notification.path.filename().string().find("_journal") == std::string::npos) //*better identifier for journal files 
            {
                journalUtils::createNewJournal(notification.path); 
                getFilesInDirectory();
            }
        }
        else if (notification.event == inotify::Event::close_write)
        {
            if (notification.path.filename().string().find("_journal") == std::string::npos)
            {
                journalUtils::updateJournal(notification.path); 
                setFileContents(); 
                mReconstructionDates = journalUtils::getReconstructionDates(mCurrentFile);
            }
        }      
    };

    // events to watch for 
    auto events = {
        inotify::Event::create,
        inotify::Event::close_write, 
    };

    // build notifier for handling events 
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


void myApp::getFilesInDirectory()
{
    mFilesInDir.clear();
    try
    {
        for (const auto& file : std::filesystem::directory_iterator(mCurrentDirPath)) 
        {
            if (isRegularFile(file)) 
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

void myApp::setFileContents()
{
    mFileContents = getFileContents(mCurrentFile); 
    std::filesystem::path journalPath = journalUtils::getJournalPath(mCurrentFile); 
    if (std::filesystem::is_regular_file(journalPath))
    {
        mJournalContents = getFileContents(journalPath); 
    }
}

bool myApp::isRegularFile(const std::filesystem::directory_entry& file)
{
    if (!file.path().filename().string().empty() && file.is_regular_file() && file.path().filename().string()[0] != '.')
        return true; 
    else 
        return false; 
}

void myApp::ShowJournalReconstruction()
{
    ImGui::SeparatorText("Reconstruct File"); 

    ImGui::PushItemWidth(150); 

    if (ImGui::BeginListBox("  ")) 
    {
        for (const auto& date : mReconstructionDates) 
        {
            if (ImGui::Selectable(date.second.c_str())) 
            {        
                mReconstructionLineNum = date.first; 
                ImGui::OpenPopup("Reconstruct?");
            }
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter(); 
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)); 

        if (ImGui::BeginPopupModal("Reconstruct?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure you want to reconstruct the selected file?"); 
            if (ImGui::Button("Confirm", ImVec2(120, 0)))
            {
                journalUtils::reconstructJournalFromSelectedDate(mCurrentFile, mReconstructionLineNum);
                setFileContents(); 
                ImGui::CloseCurrentPopup(); 
            }

            ImGui::SameLine(); 

            if (ImGui::Button("Cancel",  ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup(); 
            }

            ImGui::EndPopup();
        }

    ImGui::EndListBox();
    }

    ImGui::PopItemWidth(); 
} 
