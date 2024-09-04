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
#include "application.hpp"
#include <algorithm>
#ifdef _WIN32
#pragma comment (lib, "Dwmapi")
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <dwmapi.h>
#endif

//----------------------------------------------------------------------------
// Default constructor for the Ottocento Window. Should be called after glfwInit.
OttWindow::OttWindow(OttApplication* ApplicationInstance, const char* title, int winWidth, int winHeight, bool show)
{
        application = ApplicationInstance;
        m_width = winWidth;
        m_height = winHeight;
        winWidth  = std::clamp(winWidth, 1, glfwGetVideoMode(glfwGetPrimaryMonitor())->width);
        winHeight = std::clamp(winHeight, 1, glfwGetVideoMode(glfwGetPrimaryMonitor())->height);

        m_window = glfwCreateWindow(winWidth, winHeight, title, nullptr, nullptr);
        if (!m_window) { throw std::runtime_error("Failed to create GLFW window!"); }
        glfwSetWindowUserPointer(m_window, this);
        glfwSetWindowSizeLimits(m_window, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
        
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
        glfwSetWindowRefreshCallback(m_window, windowResizeCallback);

        glfwSetWindowRefreshCallback(m_window,
        [](GLFWwindow* glfwWindow, int width, int height) -> void {
          auto* window = reinterpret_cast<OttWindow*>(glfwGetWindowUserPointer(glfwWindow));
          window->OnWindowResized({width, height});
        });
            
        glfwSetScrollCallback(m_window, Input::scrollCallback);
        glfwSetKeyCallback(m_window, keyCallback);

        m_icon.pixels = stbi_load("src/icon.png", &m_icon.width, &m_icon.height, 0, 4);
        if (m_icon.pixels) { glfwSetWindowIcon(m_window, 1, &m_icon); }
            
        #ifdef WIN32
        ThemeRefreshDarkMode(m_window);
        #endif
}

//----------------------------------------------------------------------------
OttWindow::~OttWindow()
{
    glfwDestroyWindow(m_window);
}

//----------------------------------------------------------------------------
VkSurfaceKHR OttWindow::createSurface(VkInstance instance)
{
    VkSurfaceKHR surface  = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
    return surface;
}

//----------------------------------------------------------------------------
std::vector<const char*> OttWindow::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

//----------------------------------------------------------------------------
// Windows Specific: refresh the titlebar to DarkMode.
// This code is from a problem already solved by the Blender devs. It is extracted
// from the 'ddbac88c08' commit "Win32: Dark Mode Title Bar Color" by Harley Acheson
#if _WIN32
void OttWindow::ThemeRefreshDarkMode(GLFWwindow* WIN32_window)
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
        #ifdef NDEBUG 
        DwmSetWindowAttribute(hwnd, 20, &DarkMode, sizeof(DarkMode));
        #endif
    }

    // This is a workaround I added for the window to minimize and restore so it can display
    // the darkmode right when the GLFW window is initiated.
    glfwIconifyWindow(m_window);
    if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED))
        glfwRestoreWindow(m_window); glfwMaximizeWindow(m_window);
}
#endif