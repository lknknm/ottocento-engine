#pragma once

//----------------------------------------------------------------------------
// Input handling implementation class.
namespace Input
{
    //----------------------------------------------------------------------------
    bool isKeyDown(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    //----------------------------------------------------------------------------
    bool isKeyReleased(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_RELEASE;
    }

    //----------------------------------------------------------------------------
    bool isMouseButtonDown(GLFWwindow* windowHandle, int mButtonCode)
    {
        int state = glfwGetMouseButton(windowHandle, mButtonCode);
        return state == GLFW_PRESS;
    }

    //----------------------------------------------------------------------------
    glm::vec2 getMousePosition(GLFWwindow* windowHandle)
    {
        double x, y;
        glfwGetCursorPos(windowHandle, &x, &y);
        return {(float)x, (float)y};
    }
}; // namespace Input
