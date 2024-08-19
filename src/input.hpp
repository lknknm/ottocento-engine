#pragma once

//----------------------------------------------------------------------------
// Input handling implementation class. 
class Input
{
public:
    
    //----------------------------------------------------------------------------
    static bool isKeyDown(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    //----------------------------------------------------------------------------
    static bool isKeyReleased(GLFWwindow* windowHandle, int keyCode)
    {
        int state = glfwGetKey(windowHandle, keyCode);
        return state == GLFW_RELEASE;
    }

    //----------------------------------------------------------------------------
    static bool isMouseButtonDown(GLFWwindow* windowHandle, int mButtonCode)
    {
        int state = glfwGetMouseButton(windowHandle, mButtonCode);
        return state == GLFW_PRESS;
    }

    //----------------------------------------------------------------------------
    static glm::vec2 getMousePosition(GLFWwindow* windowHandle)
    {
        double x, y;
        glfwGetCursorPos(windowHandle, &x, &y);
        return { (float)x, (float)y };
    }
};

