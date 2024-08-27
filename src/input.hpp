#pragma once

//----------------------------------------------------------------------------
// Input handling implementation.
#include <GLFW/glfw3.h>
#include <glm.hpp>
namespace Input
{
    //----------------------------------------------------------------------------
    inline bool isKeyDown(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
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
}; // namespace Input
