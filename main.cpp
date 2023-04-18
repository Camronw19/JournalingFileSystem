#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>

#include "myApp.h"

int main()
{
	if (!glfwInit())
		return 1;

	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - example", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // tells open gl to draw to this window
        throw("Unable to context to OpenGL"); 

	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);


    myApp app; 
    app.Init(window, glsl_version); 
    
    while(!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT); 
        app.NewFrame();  
        app.Update(); 
        app.Render(); 
        glfwSwapBuffers(window);
    }

    app.Shutdown(); 

    return 0;
}