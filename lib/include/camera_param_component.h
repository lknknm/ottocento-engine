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

struct OttCameraParametersComponent {
    double resetAnimationStart{0.};
    double orbitAnimationStart{0.};
    
    float VerticalFOV = 38.0f;
    constexpr float NearClip = 0.1f;
    constexpr float FarClip = 1000.0f;
    constexpr float speed = 2.0f;
    constexpr float orthoZoomFactor = 10.f;
    constexpr float rotationSpeed = 0.3f;
};