#include "myApp.h"


void myApp::Init(GLFWwindow* window, const char* glsl_version)
{
    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext(); 
    ImGuiIO &io = ImGui::GetIO(); 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version); 
    ImGui::StyleColorsDark(); 
}

void myApp::Update() 
{
    ImGui::Begin("Test");
    ImGui::Text("Hello World!"); 
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