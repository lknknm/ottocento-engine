#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "window.h"
#include <glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera;
//----------------------------------------------------------------------------
// Input handling implementation class.
// State timeline cheatsheet, extracted from StackOverflow https://stackoverflow.com/a/37195173:
//----------------------------------------------------------------------------
// state                  released               pressed                released
// timeline             -------------|------------------------------|---------------
//                                   ^                              ^
// key callback calls           GLFW_PRESS                    GLFW_RELEASE
//----------------------------------------------------------------------------

namespace Input
{
    //----------------------------------------------------------------------------
    inline bool isKeyDownRepeat(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    
    //----------------------------------------------------------------------------
    inline bool isKeyDown(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_PRESS ;
    }

    //----------------------------------------------------------------------------
    inline bool isKeyReleased(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_RELEASE;
    }

    //----------------------------------------------------------------------------
    inline bool isMouseButtonDown(GLFWwindow* windowHandle, int mButtonCode)
    {
        int state = glfwGetMouseButton(windowHandle, mButtonCode);
        return state == GLFW_PRESS;
    }

    //----------------------------------------------------------------------------
    inline glm::vec2 getMousePosition(GLFWwindow* windowHandle)
    {
        double x, y;
        glfwGetCursorPos(windowHandle, &x, &y);
        return {(float)x, (float)y};
    }

    //----------------------------------------------------------------------------
    inline double yoffsetCallback = 0.0f; 
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        yoffsetCallback += yoffset;
    }

    // //----------------------------------------------------------------------------
    // void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    // {
    //     if (action == GLFW_RELEASE)
    //     {
    //         std::cout << "Key released: " << key << std::endl;
    //         // Take action here
    //     }
    // }
    
} // namespace Input
