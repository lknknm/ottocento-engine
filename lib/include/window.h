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
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <volk.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>
#include <glm/vec2.hpp>

struct GLFWwindow;
using VkInstance   = struct VkInstance_T*;
using VkSurfaceKHR = struct VkSurfaceKHR_T*;

#ifdef NDEBUG
constexpr inline bool enableValidationLayers = false;
#else
constexpr inline bool enableValidationLayers = true;
#endif

// OttWindow Class is a wrapper around the GLFWwindow.
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
    std::function<void(int key, int scancode, int action, int mods)> keyCallback;

    // Soon to be transferred to the Window Manager.
    bool windowShouldClose() const { return glfwWindowShouldClose(m_window); }
    static void waitEvents()       { return glfwWaitEvents(); }
    static void update()           { return glfwPollEvents(); }
    void ThemeRefreshDarkMode(GLFWwindow* WIN32_window) const;

//----------------------------------------------------------------------------
private:
    GLFWwindow* m_window = nullptr;
    GLFWimage m_icon{};
    
    int m_width  = 0;
    int m_height = 0;
    
    //----------------------------------------------------------------------------
    static void windowTerminate() { return glfwTerminate();  }
}; // class OttWindow