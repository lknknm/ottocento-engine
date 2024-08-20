#pragma once
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "input.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>

class Camera
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    GLFWwindow* windowHandle = nullptr;

    //----------------------------------------------------------------------------
    // Recalculate view with input handling functions
    glm::mat4 recalculateView()
    {
        glm::vec2 mousePos = Input::getMousePosition(windowHandle);
        glm::vec2 delta = (mousePos - lastMousePosition) * 0.002f;
        lastMousePosition = mousePos;
        
        forwardDirection = CenterPosition - EyePosition;
        rightVector = glm::cross(forwardDirection, upVector);
        
        if (!Input::isMouseButtonDown(windowHandle, GLFW_MOUSE_BUTTON_RIGHT))
        {
            glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            return glm::lookAt(EyePosition, CenterPosition, upVector);
        }
        glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + 2);
        
        if (windowHandle != nullptr && Input::isKeyDown(windowHandle, GLFW_KEY_E))
            moveUpDirection();
        if (windowHandle != nullptr && Input::isKeyDown(windowHandle, GLFW_KEY_Q))
            moveDownDirection();
        if (windowHandle != nullptr && Input::isKeyDown(windowHandle, GLFW_KEY_W))
            moveForward();
        if (windowHandle != nullptr && Input::isKeyDown(windowHandle, GLFW_KEY_S))
            moveBack();
        if (windowHandle != nullptr && Input::isKeyDown(windowHandle, GLFW_KEY_D))
            moveRightDirection();
        if (windowHandle != nullptr && Input::isKeyDown(windowHandle, GLFW_KEY_A))
            moveLeftDirection();

        if (delta.x != 0.0f || delta.y != 0.0f)
        {
            float pitchDelta = delta.y * rotationSpeed;
            float yawDelta = delta.x * rotationSpeed;

            glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightVector), glm::angleAxis(-yawDelta, upVector)));
            forwardDirection = glm::rotate(q, forwardDirection);
            CenterPosition = EyePosition + forwardDirection;
            EyePosition = CenterPosition - forwardDirection;
        }
        
        return glm::lookAt(EyePosition, CenterPosition, upVector);
    }
    
//----------------------------------------------------------------------------
// Projection and Direction functions
//----------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------
    glm::mat4 perspectiveProjection(uint32_t width, uint32_t height)
    {
        return glm::perspective(glm::radians(VerticalFOV), width / (float) height, NearClip, FarClip);
    }

    //----------------------------------------------------------------------------
    void moveUpDirection()
    {
        CenterPosition += upVector * speed;
        EyePosition += upVector * speed;
    }

    //----------------------------------------------------------------------------
    void moveDownDirection()
    {
        CenterPosition -= upVector * speed;
        EyePosition -= upVector * speed;
    }
    
    //----------------------------------------------------------------------------
    void moveForward()
    {
        CenterPosition += forwardDirection * speed;
        EyePosition += forwardDirection * speed;
    }

    //----------------------------------------------------------------------------
    void moveBack()
    {
        CenterPosition -= forwardDirection * speed;
        EyePosition -= forwardDirection * speed;
    }

    //----------------------------------------------------------------------------
    void moveRightDirection()
    {
        CenterPosition += rightVector * speed;
        EyePosition += rightVector * speed;
    }
    
    //----------------------------------------------------------------------------
    void moveLeftDirection()
    {
        CenterPosition -= rightVector * speed;
        EyePosition -= rightVector * speed;
    }
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    float VerticalFOV = 45.0f;
    float NearClip = 0.1f;
    float FarClip = 100.0f;
    float speed = 0.0001f;
    float rotationSpeed = 0.3f;

    glm::vec2 lastMousePosition{ 0.0f, 0.0f };
    
    glm::vec3 CenterPosition{0.0f, 0.0f, 0.0f};
    glm::vec3 EyePosition{2.0f, 2.0f, 2.0f};
    glm::vec3 rightVector{0.0f, 0.0f, 0.0f};
    glm::vec3 upVector{0.0f, 0.0f, 1.0f};
    glm::vec3 forwardDirection{};
    
    glm::mat4 Projection{ 1.0f };
    glm::mat4 View{ 1.0f };
    glm::mat4 InverseProjection{ 1.0f };
    glm::mat4 InverseView{ 1.0f };
};
