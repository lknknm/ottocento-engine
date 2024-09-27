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

#include "window.h"
#include "input.hxx"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WIN32
#pragma comment (lib, "Dwmapi")
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#endif

//----------------------------------------------------------------------------
// Default constructor for the Ottocento Window.
// Initiate GLFW window with specific parameters and sets up the window icon.
// Windows-specific: Refresh window to darkmode.
OttWindow::OttWindow(const char* title, int winWidth, int winHeight, bool show)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
        m_width = winWidth; m_height = winHeight;
    
        m_window = glfwCreateWindow(winWidth, winHeight, title, nullptr, nullptr);
        if (!m_window) { throw std::runtime_error("Failed to create GLFW window!"); }
        glfwSetWindowUserPointer(m_window, this);
        glfwSetWindowSizeLimits(m_window, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
        
        glfwSetFramebufferSizeCallback(m_window,
        [](GLFWwindow* window, int width, int height) -> void
        {
          auto* windowPtr = reinterpret_cast<OttWindow*>(glfwGetWindowUserPointer(window));
          windowPtr->OnFramebufferResized({width, height});
        });
    
        glfwSetWindowRefreshCallback(m_window,
        [](GLFWwindow* window) -> void
        {
          auto* windowPtr = reinterpret_cast<OttWindow*>(glfwGetWindowUserPointer(window));
          windowPtr->OnWindowRefreshed();
        });

        glfwSetDropCallback(m_window,
        [](GLFWwindow* window, int count, const char** paths) -> void
        {
            auto* windowPtr = reinterpret_cast<OttWindow*>(glfwGetWindowUserPointer(window));
            windowPtr->OnFileDropped(count, paths);
        });
            
        glfwSetScrollCallback(m_window, Input::scrollCallback);
        glfwSetKeyCallback(m_window, keyCallback);

        m_icon.pixels = stbi_load("resource/icon.png", &m_icon.width, &m_icon.height, 0, 4);
        if (m_icon.pixels) { glfwSetWindowIcon(m_window, 1, &m_icon); }
}

//----------------------------------------------------------------------------
VkSurfaceKHR OttWindow::createWindowSurface(VkInstance instance)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
    return surface;
}

//----------------------------------------------------------------------------
glm::ivec2 OttWindow::getFrameBufferSize() const
{
    glm::ivec2 fbSize(0, 0);
    glfwGetFramebufferSize(m_window, &fbSize.x, &fbSize.y);
    return fbSize;
}

//----------------------------------------------------------------------------
void OttWindow::getCursorPos(double* xpos, double* ypos) const
{
    glfwGetCursorPos(m_window, xpos, ypos);
}

//----------------------------------------------------------------------------
void OttWindow::setCursorPos(double xpos, double ypos) const
{
    glfwSetCursorPos(m_window, xpos, ypos);
}

//----------------------------------------------------------------------------
OttWindow::~OttWindow()
{
    glfwDestroyWindow(m_window);
    stbi_image_free(m_icon.pixels);
    glfwTerminate();
}

//----------------------------------------------------------------------------
// Windows Specific: refresh the titlebar to DarkMode.
// This code is from a problem already solved by the Blender devs. It is extracted
// from the 'ddbac88c08' commit "Win32: Dark Mode Title Bar Color" by Harley Acheson
#if _WIN32
void OttWindow::ThemeRefreshDarkMode(GLFWwindow* WIN32_window) const
{
    DWORD lightMode;
    DWORD pcbData = sizeof(lightMode);
    HWND hwnd = glfwGetWin32Window(WIN32_window);
    if (RegGetValueW(HKEY_CURRENT_USER,
             L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize\\",
             L"AppsUseLightTheme",
             RRF_RT_REG_DWORD,
             NULL,
             &lightMode,
             &pcbData) == ERROR_SUCCESS)
    {
        BOOL DarkMode = !lightMode;
        /* 20 == DWMWA_USE_IMMERSIVE_DARK_MODE in Windows 11 SDK.  This value was undocumented for
        * Windows 10 versions 2004 and later, supported for Windows 11 Build 22000 and later. */
        // This function is being broken on Debug mode so I'll just leave it for release builds.
        DwmSetWindowAttribute(hwnd, 20, &DarkMode, sizeof(DarkMode));
    }
    glfwIconifyWindow(m_window);
    if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED))
        glfwRestoreWindow(m_window);
    glfwMaximizeWindow(m_window);
}
#endif
