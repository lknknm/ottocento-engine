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

struct OttCameraSystem
{
//----------------------------------------------------------------------------
public:
    glm::mat4 recalculateView(float deltaTime);
        
//----------------------------------------------------------------------------
// Projection and Direction functions
//----------------------------------------------------------------------------
    
    glm::mat4 projection(float height, float width) const;
    glm::mat4 inverseProjection(glm::mat4 perspectiveProjection, glm::mat4 view);

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
}; // class OttCameraSystem