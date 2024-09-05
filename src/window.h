// Ottocento Engine. Architectural BIM Engine.
// Copyright (C) 2024  Lucas M. Faria.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>
#include <vec2.hpp>
#include <vector>

struct GLFWwindow;
using VkInstance   = struct VkInstance_T*;
using VkSurfaceKHR = struct VkSurfaceKHR_T*;


// OttWindow Class is an abstraction on top of the GLFWwindow.
// It defines window specific functions and callbacks to pass them to other classes.
class OttWindow
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    OttWindow(const char* title, int width, int height, bool show = true);
    ~OttWindow();
    
//----------------------------------------------------------------------------
// Getters and setters
//----------------------------------------------------------------------------
    GLFWwindow* getWindowhandle() const { return this->m_window; }
    glm::ivec2 getFrameBufferSize() const;
    
    void getCursorPos(double* xpos, double* ypos) const;
    void setCursorPos(double xpos, double ypos) const;

//----------------------------------------------------------------------------
// Callbacks and glfw specific setup
//----------------------------------------------------------------------------
    std::function<void()> OnWindowRefreshed;
    std::function<void(glm::vec2)> OnFramebufferResized;
    std::function<void(int count, const char** paths)> OnFileDropped;

    // Soon to be transfered to the Window Manager.
    bool windowShouldClose() const { return glfwWindowShouldClose(m_window); }
    static void waitEvents()      { return glfwWaitEvents(); }
    static void update()          { return glfwPollEvents(); }
    void ThemeRefreshDarkMode(GLFWwindow* WIN32_window) const;
    VkSurfaceKHR createWindowSurface(VkInstance instance);
    
    //----------------------------------------------------------------------------
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_RELEASE)
        {
            std::cout << "Key released: " << key << std::endl;
            //Take action here
        }
    }

//----------------------------------------------------------------------------
private:
    GLFWwindow* m_window = nullptr;
    GLFWimage m_icon{};
    
    int m_width = 0.0f;
    int m_height = 0.0f;
    
    //----------------------------------------------------------------------------
    static void windowTerminate() { return glfwTerminate();  }
}; // class OttWindow