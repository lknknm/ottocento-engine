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
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <iostream>

#include "input.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>

class OttCamera
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    GLFWwindow* windowHandle = nullptr;

    //----------------------------------------------------------------------------
    // Recalculate view with input handling functions
    glm::mat4 recalculateView(float deltaTime)
    {
        if (Input::isKeyDown(windowHandle, GLFW_KEY_F3))
        {
            // walkNavigation = !walkNavigation;
            glfwWaitEventsTimeout(1.0f);
            std::cout << "walkNavigation: " << walkNavigation << std::endl;
            std::cout << "----------------" << std::endl;
        }
        if (!walkNavigation)
            viewportInputHandle(deltaTime);
        else
            walkNavigationInputHandle(deltaTime);
        
        return glm::lookAt(EyePosition, CenterPosition, upVector);
    }
        
//----------------------------------------------------------------------------
// Projection and Direction functions
//----------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------
    glm::mat4 perspectiveProjection(float aspectRatio)
    {
        return glm::perspective(glm::radians(VerticalFOV), aspectRatio, NearClip, FarClip);
    }

    //----------------------------------------------------------------------------
    glm::mat4 inverseProjection(glm::mat4 perspectiveProjection, glm::mat4 view)
    {
        return glm::inverse(perspectiveProjection * view);
    }

//----------------------------------------------------------------------------
// Getters
//----------------------------------------------------------------------------
    //----------------------------------------------------------------------------
    glm::vec3 getEyePosition()
    {
        return EyePosition;
    }
    
    //----------------------------------------------------------------------------
    glm::vec3 getCenterPosition()
    {
        return CenterPosition;
    }

    //----------------------------------------------------------------------------
    glm::vec3 getEnvironmentUpVector()
    {
        return upVector;
    }
    
    //----------------------------------------------------------------------------
    // "Redundant" name for it not to be mistaken with the world upVector.
    glm::vec3 getCameraUpVector()
    {
        return glm::cross(forwardDirection, -rightVector);
    }

    //----------------------------------------------------------------------------
    // "Redundant" name for it not to be mistaken with the world rightVector.
    glm::vec3 getCameraRightVector()
    {
        return glm::cross(forwardDirection, upVector);
    }

    glm::mat4 getViewMatrix()
    {
        return ViewMatrix;
    }
    
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    double resetAnimationStart = 0;
    double orbitAnimationStart = 0;
    
    float VerticalFOV = 38.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    float speed = 2.0f;
    float rotationSpeed = 0.3f;
    bool  walkNavigation = false;
    
    glm::vec2 lastMousePosition{ 0.0f, 0.0f };
    
    glm::vec3 CenterPosition{0.0f, 0.0f, 0.0f};
    
    glm::vec3 EyePosition{5.0f, -5.0f, 5.0f};
    glm::vec3 startEye, startCenter, targetEyePosition, targetCenterPosition;
    
    glm::vec3 rightVector{0.0f, 0.0f, 0.0f};
    glm::vec3 upVector{0.0f, 0.0f, 1.0f};
    glm::vec3 forwardDirection{};
    
    glm::mat4 Projection{ 1.0f };
    glm::mat4 ViewMatrix{ 1.0f };
    glm::mat4 InverseProjection{ 1.0f };
    glm::mat4 InverseView{ 1.0f };

    enum struct ViewType
    {
        VT_FRONT,
        VT_BACK,
        VT_RIGHT,
        VT_LEFT,
        VT_TOP,
        VT_BOTTOM,
        VT_ISOMETRIC,
        VT_INVERT_ISOMETRIC
    };
    
    enum struct rotateDirection
    {
        RD_RIGHT,
        RD_LEFT,
        RD_UP,
        RD_DOWN
    };

    
    //----------------------------------------------------------------------------
    // This function handles all the hotkeys for the viewport camera implementation.
    // It uses the Input namespace abstraction to handle GLFW interaction.
    void viewportInputHandle(float deltaTime)
    {
        glm::vec2 mousePos = Input::getMousePosition(windowHandle);
        glm::vec2 delta = (mousePos - lastMousePosition) * 0.002f;
        lastMousePosition = mousePos;
        
        forwardDirection = CenterPosition - EyePosition;
        rightVector = glm::cross(forwardDirection, upVector);

        if(windowHandle != nullptr )
        {
            if (!Input::isMouseButtonDown(windowHandle, GLFW_MOUSE_BUTTON_MIDDLE))
            {
                glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_4))
                    rotateFixedAmount(rotateDirection::RD_LEFT);
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_6))
                    rotateFixedAmount(rotateDirection::RD_RIGHT);
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_8))
                    rotateFixedAmount(rotateDirection::RD_UP);
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_2))
                    rotateFixedAmount(rotateDirection::RD_DOWN);
                
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_0))
                    resetToInitialPos();
                
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_1))
                    Input::isKeyDownRepeat(windowHandle, GLFW_KEY_LEFT_CONTROL) ?
                        orbitStartAnimation(ViewType::VT_BACK) :
                        orbitStartAnimation(ViewType::VT_FRONT);
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_3))
                    Input::isKeyDownRepeat(windowHandle, GLFW_KEY_LEFT_CONTROL) ?
                        orbitStartAnimation(ViewType::VT_LEFT) :
                        orbitStartAnimation(ViewType::VT_RIGHT);
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_7))
                    Input::isKeyDownRepeat(windowHandle, GLFW_KEY_LEFT_CONTROL) ?
                        orbitStartAnimation(ViewType::VT_BOTTOM) :
                        orbitStartAnimation(ViewType::VT_TOP);
                if (Input::isKeyDown(windowHandle, GLFW_KEY_KP_9))
                    Input::isKeyDownRepeat(windowHandle, GLFW_KEY_LEFT_CONTROL) ?
                        orbitStartAnimation(ViewType::VT_INVERT_ISOMETRIC) :
                        orbitStartAnimation(ViewType::VT_ISOMETRIC);
                
                animateResetUpdate();

                if (Input::yoffsetCallback > 0)
                    zoomIn(Input::yoffsetCallback); 
                if (Input::yoffsetCallback < 0)
                    zoomOut(Input::yoffsetCallback); 
                Input::yoffsetCallback = 0;
                
                return;
            }

            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_LEFT_SHIFT))
            {
                if (delta.x != 0.0f || delta.y != 0.0f)
                {
                    EyePosition      -= rightVector * delta.x * 0.3f; 
                    CenterPosition   -= rightVector * delta.x * 0.3f;
                    EyePosition      += getCameraUpVector() * delta.y * 0.3f / (glm::distance(EyePosition,CenterPosition));
                    CenterPosition   += getCameraUpVector() * delta.y * 0.3f / (glm::distance(EyePosition,CenterPosition));
                    return;
                }
            }
            
            if (delta.x != 0.0f || delta.y != 0.0f && !Input::isKeyDown(windowHandle, GLFW_KEY_LEFT_SHIFT))
            {
                float pitchDelta = delta.y * rotationSpeed;
                float yawDelta = delta.x * speed;
                
                glm::quat qPitch = glm::angleAxis(-pitchDelta, rightVector);
                glm::quat qYaw = glm::angleAxis(-yawDelta, glm::vec3(0.0f, 0.0f, upVector.z));
                glm::mat4 rotationMatrix = glm::toMat4(glm::normalize(glm::cross(qYaw, qPitch)));
                
                forwardDirection = rotationMatrix * glm::vec4(forwardDirection, 0.0f);
                upVector = glm::normalize(rotationMatrix * glm::vec4(upVector, 0.0f));
                EyePosition = CenterPosition - forwardDirection;
            }
        }
    }

    //----------------------------------------------------------------------------
    // This function handles all the hotkeys for the walk navigation camera implementation.
    // It uses the Input namespace abstraction to handle GLFW interaction.
    void walkNavigationInputHandle(float deltaTime)
    {
        glm::vec2 mousePos = Input::getMousePosition(windowHandle);
        glm::vec2 delta = (mousePos - lastMousePosition) * 0.002f;
        lastMousePosition = mousePos;

        forwardDirection = CenterPosition - EyePosition;
        rightVector = glm::cross(forwardDirection, upVector);

        if(windowHandle != nullptr )
        {
            if (!Input::isMouseButtonDown(windowHandle, GLFW_MOUSE_BUTTON_RIGHT))
            {
                glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                return;
            }
            glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + 2);
            
            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_E))
                moveUpDirection(deltaTime);
            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_Q))
                moveDownDirection(deltaTime);
            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_W))
                moveForward(deltaTime);
            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_S))
                moveBack(deltaTime);
            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_D))
                moveRightDirection(deltaTime);
            if (Input::isKeyDownRepeat(windowHandle, GLFW_KEY_A))
                moveLeftDirection(deltaTime);

            if (delta.x != 0.0f || delta.y != 0.0f)
            {
                float pitchDelta = delta.y * rotationSpeed;
                float yawDelta = delta.x * rotationSpeed;

                glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightVector), glm::angleAxis(-yawDelta, upVector)));
                forwardDirection = glm::rotate(q, forwardDirection);
                CenterPosition = EyePosition + forwardDirection;
                EyePosition = CenterPosition - forwardDirection;
            }
        }
    }
    
    // ----------------------------------------------------------------------------
    // This function uses the same formula principles as the viewportInputHandle function
    // for the mouse movement. The degrees for movement are calculated by multiplying:
    // SIDEWAYS: angle * rotationSpeed * smoothness scalar.
    // UP/DOWN:  angle * rotationSpeed * smoothness scalar / distance to the center of the camera.
    // Scalars are defined arbitrarily and not assigned in order to save up some memory reads.
    void rotateFixedAmount(rotateDirection direction)
    {
        float degreesRight = 0.0f;
        float degreesUp = 0.0f;
        switch (direction)
        {
        case rotateDirection::RD_LEFT:
            degreesRight = -5.0f * rotationSpeed * 0.2f;
            break;
        case rotateDirection::RD_RIGHT:
            degreesRight = 5.0f * rotationSpeed * 0.2f;
            break;
        case rotateDirection::RD_UP:
            degreesUp = 1.0f * rotationSpeed * 0.5f / (glm::distance(EyePosition,CenterPosition));
            break;
        case rotateDirection::RD_DOWN:
            degreesUp = -1.0f * rotationSpeed * 0.5f / (glm::distance(EyePosition,CenterPosition));
            break;
        }

        glm::quat qPitch = glm::angleAxis(-glm::radians(degreesUp), rightVector);
        glm::quat qYaw = glm::angleAxis(glm::radians(degreesRight), glm::vec3(0.0f, 0.0f, upVector.z));
        glm::mat4 rotationMatrix = glm::toMat4(glm::normalize(glm::cross(qYaw, qPitch)));
                
        forwardDirection = rotationMatrix * glm::vec4(forwardDirection, 0.0f);
        upVector = glm::normalize(rotationMatrix * glm::vec4(upVector, 0.0f));
        EyePosition = CenterPosition - forwardDirection;
    }
    
    //----------------------------------------------------------------------------
    // Originally implemented on F3D with the help of
    // Michael Migliore, Mathieu Westphal and Snoyer.
    // This function takes the current position of the camera and reorient it facing
    // a given axis, as per the formula:
    // P' = P + radius * viewAxis.
    glm::vec3 SetViewOrbit(ViewType view)
    {
        glm::vec3 foc = getCenterPosition();
        glm::vec3 pos = getEyePosition();
        glm::vec3 up  = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 axis, newPos;
        float radius = distance(foc, pos);

        switch (view)
        {
        case ViewType::VT_FRONT:
            axis = { 0, +1, 0 };
            break;
        case ViewType::VT_BACK:
            axis = { 0, -1, 0 };
            break;
        case ViewType::VT_RIGHT:
            axis = { +1, 0, 0 };
            break;
        case ViewType::VT_LEFT:
            axis = { -1, 0, 0 };
            break;
        case ViewType::VT_TOP:
            axis = { 0, 0, +1 };
            up   = { 0, -1, 0 };
            break;
        case ViewType::VT_BOTTOM:
            axis = { 0, 0, -1 };
            up   = { 0, -1, 0 };
            break;
        case ViewType::VT_ISOMETRIC:
            axis = { -1, +1, +1 };
            break;
        case ViewType::VT_INVERT_ISOMETRIC:
            axis = { -1, -1, -1 };
            break;
        }
        
        newPos[0] = foc[0] + glm::clamp(radius, 0.0f, 100.0f) * axis[0];
        newPos[1] = foc[1] + glm::clamp(radius, 0.0f, 100.0f) * axis[1];
        newPos[2] = foc[2] + glm::clamp(radius, 0.0f, 100.0f) * axis[2];
        
        upVector = up;
        return newPos;
    }


    //----------------------------------------------------------------------------
    // Resets the timer and consequently starts the animation to reposition the camera
    // to its initial state of the 0, 0, 0 center position.
    void resetToInitialPos()
    {
        if((float)resetAnimationStart == 0.0f)
        {
            upVector = {0.0f, 0.0f, 1.0f};
            startCenter = getCenterPosition();
            startEye = getEyePosition();
            targetCenterPosition = glm::vec3(0.0f,  0.0f, 0.0f);
            targetEyePosition    = glm::vec3(5.0f, -5.0f, 5.0f);
            resetAnimationStart  = glfwGetTime();
        }
    }
    
    //----------------------------------------------------------------------------
    // Resets the timer and consequently starts the animation for the implementation of
    // FRONT, RIGHT, TOP (etc...) views, defined in SetViewOrbit(view).
    void orbitStartAnimation(ViewType view)
    {
        if((float)resetAnimationStart == 0.0f)
        {
            startCenter = getCenterPosition();
            targetCenterPosition = startCenter;
            startEye = getEyePosition();
            targetEyePosition = SetViewOrbit(view);
            resetAnimationStart  = glfwGetTime();
        }
    }

    //----------------------------------------------------------------------------
    // Updates each frame with a new position based on time per each frame.
    // The animation will stop to its final position and then reset the timer.
    // Smoothness scalars represented with an arbitrary value, not assigned to a variable
    // to save up some memory reads.
    void animateResetUpdate()
    {
        if (resetAnimationStart > 0)
        {
            const double timeSinceStart = glfwGetTime() - resetAnimationStart;
            const double t = glm::min((float)(timeSinceStart / (rotationSpeed * 0.2)), 1.0f);
                
            CenterPosition = glm::mix(startCenter, targetCenterPosition, t);
            EyePosition = glm::mix(startEye, targetEyePosition, t);

            if (timeSinceStart >= (rotationSpeed * 0.2)) { resetAnimationStart = 0; }
        }
    }
    
    //----------------------------------------------------------------------------
    void moveUpDirection(float deltaTime)
    {
        CenterPosition += upVector * speed * deltaTime;
        EyePosition += upVector * speed * deltaTime;
    }

    //----------------------------------------------------------------------------
    void moveDownDirection(float deltaTime)
    {
        CenterPosition -= upVector * speed * deltaTime;
        EyePosition -= upVector * speed * deltaTime;
    }
    
    //----------------------------------------------------------------------------
    void moveForward(float deltaTime)
    {
        CenterPosition += forwardDirection * speed * deltaTime;
        EyePosition += forwardDirection * speed * deltaTime;
    }

    //----------------------------------------------------------------------------
    void moveBack(float deltaTime)
    {
        CenterPosition -= forwardDirection * speed * deltaTime;
        EyePosition -= forwardDirection * speed * deltaTime;
    }

    //----------------------------------------------------------------------------
    void moveRightDirection(float deltaTime)
    {
        CenterPosition += rightVector * speed * deltaTime;
        EyePosition += rightVector * speed * deltaTime;
    }
    
    //----------------------------------------------------------------------------
    void moveLeftDirection(float deltaTime)
    {
        CenterPosition -= rightVector * speed * deltaTime;
        EyePosition -= rightVector * speed * deltaTime;
    }

    //----------------------------------------------------------------------------
    void zoomIn(double yoffset)
    {
        EyePosition += forwardDirection * 0.2f;
    }

    //----------------------------------------------------------------------------
    void zoomOut(double yoffset)
    {
        EyePosition -= forwardDirection * 0.2f;
    }
};
