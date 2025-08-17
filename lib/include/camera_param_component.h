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

struct OttCameraParametersComponent 
{
    enum ProjectionType : uint8_t {
        Perspective,
        Orthographic
    };
    
    double resetAnimationStart{0.};
    double orbitAnimationStart{0.};
    
    float VerticalFOV{38.0f};
    float NearClip{0.1f};
    float FarClip{1000.0f};
    float speed{2.0f};
    float orthoZoomFactor{10.f};
    float rotationSpeed{0.3f};
};

