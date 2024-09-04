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
#include <iostream>
#include <vec2.hpp>
#include <vector>

struct GLFWwindow;
using VkInstance   = struct VkInstance_T*;
using VkSurfaceKHR = struct VkSurfaceKHR_T*;
typedef void (*Delegate)();

class OttApplication;

// OttWindow Class is an abstraction on top of the GLFWwindow.
// It defines window specific functions and callbacks to pass them to the application.
class OttWindow
{
//----------------------------------------------------------------------------
public:
    OttWindow(OttApplication* ApplicationInstance, const char* title, int width, int height, bool show = true);
    ~OttWindow();
    bool framebufferResized = false;

    GLFWwindow* getWindowhandle() const { return this->m_window; }
    void getFrameBufferSize(int* width, int* height) { OttWindow* window = this; width  = &this->m_width; height = &this->m_height; }
    VkSurfaceKHR createSurface(VkInstance instance);
    std::vector<const char*> getRequiredExtensions();

    Delegate<void(glm::vec2)> OnWindowResized;

    // Soon to be transfered to the Window Manager.
    void waitEvents() { return glfwWaitEvents(); }
    void update() { return glfwPollEvents(); }
    bool windowShouldClose() const { return glfwWindowShouldClose(m_window); }
    void windowTerminate() const { return glfwTerminate(); }
    
    //----------------------------------------------------------------------------
    static void windowResizeCallback(GLFWwindow* window)
    {
        auto ptr = reinterpret_cast<OttWindow*>(glfwGetWindowUserPointer(window));
        //vkDeviceWaitIdle(app->device);

        // Recreate the swap chain with the new extent
        application->recreateSwapChain();
        application->updateUniformBufferCamera(application->currentFrame, 1, application->swapChainExtent.width, application->swapChainExtent.height);
        application->drawFrame();
    }
    
    //----------------------------------------------------------------------------
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto ptr = reinterpret_cast<OttApplication*>(glfwGetWindowUserPointer(window));
        application->framebufferResized = true;
        application->swapChainExtent.width = width;
        application->swapChainExtent.height = height;
    }
    
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
    static void dropFilesCallback(GLFWwindow* window, int path_count, const char* paths[])
    {
        auto ptr = reinterpret_cast<OttWindow*>(glfwGetWindowUserPointer(window));
        std::vector<std::string> filePaths(paths, paths + path_count);
        //ptr->dropFilesInternal(filePaths);
    }



private:
//----------------------------------------------------------------------------
    GLFWwindow* m_window = nullptr;
    OttApplication* application;
    
    int m_width = 0.0f;
    int m_height = 0.0f;
    GLFWimage m_icon{};
    
    void ThemeRefreshDarkMode(GLFWwindow* WIN32_window);
};