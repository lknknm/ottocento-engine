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

#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class OttCamera
{
//----------------------------------------------------------------------------
public:
    GLFWwindow* windowHandle = nullptr;
    OttWindow* appwindow = nullptr;
    glm::mat4 recalculateView(float deltaTime);
        
//----------------------------------------------------------------------------
// Projection and Direction functions
//----------------------------------------------------------------------------
    
    glm::mat4 projection(float height, float width) const;
    glm::mat4 inverseProjection(glm::mat4 perspectiveProjection, glm::mat4 view);

//----------------------------------------------------------------------------
// Getters
//----------------------------------------------------------------------------

    glm::vec3 getEyePosition() const { return EyePosition; }
    glm::vec3 getCenterPosition() const { return CenterPosition; }
    glm::vec3 getEnvironmentUpVector() const { return upVector; }
    
    // "Redundant" name for it not to be mistaken with the world upVector.
    glm::vec3 getCameraUpVector() const { return glm::cross(forwardDirection, -rightVector); }
    
    // "Redundant" name for it not to be mistaken with the world rightVector.
    glm::vec3 getCameraRightVector() const { return glm::cross(forwardDirection, upVector); }
    glm::mat4 getViewMatrix() const { return ViewMatrix; }

    bool getRenderState() const { return render; }
    
//----------------------------------------------------------------------------
private:
    double resetAnimationStart = 0;
    double orbitAnimationStart = 0;
    
    float VerticalFOV = 38.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    float speed = 2.0f;
    float orthoZoomFactor = 10.f;
    float rotationSpeed = 0.3f;
    bool  walkNavigation = false;
    bool  perspective = true;
    bool  render = true;
    
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
    glm::vec3 SetViewOrbit(ViewType view);
    void viewportInputHandle(float deltaTime);
    void walkNavigationInputHandle(float deltaTime);
    void rotateFixedAmount(rotateDirection direction);
    void resetToInitialPos();
    void orbitStartAnimation(ViewType view);
    void animateResetUpdate();
    void wrapAroundMousePos(glm::vec2& mousePos);
    
    //----------------------------------------------------------------------------
    void moveUpDirection(float deltaTime);
    void moveDownDirection(float deltaTime);
    void moveForward(float deltaTime);
    void moveBack(float deltaTime);
    void moveRightDirection(float deltaTime);
    void moveLeftDirection(float deltaTime);
    void zoomIn(double yoffset);
    void zoomOut(double yoffset);
}; // class OttCamera