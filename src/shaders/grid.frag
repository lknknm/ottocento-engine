#version 450

layout(location = 0) in vec2 fragCoords;
layout(location = 0) out vec4 outColor;

const float gridSize                = 100.0f;

const float cellSize                = 10.0f;
const float halfcellSize            = cellSize * 0.5f;
const float subcellSize             = 1.0f;
const float halfsubcellSize         = subcellSize * 0.5f;

const float minFadeDistance = gridSize * 0.05f;
const float maxFadeDistance = 50.0f;
float opacityFalloff;

const vec3 cameraPosition           = vec3(1.0f); // Leaving this as is for now. Still have to bind it to host.
const vec2 cellLineThickness        = vec2(0.01f);
const vec2 subcellLineThickness     = vec2(0.005f);
const vec4 cellColor                = vec4( 0.75f, 0.75f, 0.75f, 0.25f );
const vec4 subcellColor             = vec4( 0.5f, 0.5f, 0.5f, 0.25f );

vec2 cellCoords         = (mod(fragCoords, cellSize));
vec2 subcellCoords      = (mod(fragCoords, subcellSize));

vec2  distanceToCell    = abs(cellCoords);
vec2  distanceToSubcell = abs(subcellCoords);

//float ddMax(float a)
//{
//    return max(abs(dFdx(a), abs(dFdy(a))));
//}

void main() {
    vec2 dd = vec2(fwidth(cellCoords.x), fwidth(cellCoords.y));
    vec2 lineAA = dd * 1.5;
    
    vec2 adjustedCellLineThickness    = vec2(0.5f * (cellLineThickness.x + dd.x), 0.5f * (cellLineThickness.y + dd.y));
    vec2 adjustedSubcellLineThickness = vec2(0.5f * (subcellLineThickness.x + dd.x), 0.5f * (subcellLineThickness.y + dd.y));
    
    outColor = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    if (distanceToSubcell.x < adjustedSubcellLineThickness.x * 0.5 || distanceToSubcell.y < adjustedSubcellLineThickness.y * 0.5 )
        outColor = subcellColor;
    if (distanceToCell.x < adjustedCellLineThickness.x * 0.5 || distanceToCell.y < adjustedCellLineThickness.y * 0.5 )
        outColor = cellColor;
    if (fragCoords.y > -adjustedCellLineThickness.y * 0.5 && fragCoords.y < adjustedCellLineThickness.y * 0.5 )
        outColor = vec4(0.0f, 1.0f, 0.0f, 0.50f);
    if (fragCoords.x > -adjustedCellLineThickness.x * 0.5 && fragCoords.x < adjustedCellLineThickness.x * 0.5 )
        outColor = vec4(1.0f, 0.0f, 0.0f, 0.50f);
    
    float distanceToCamera = length(fragCoords - cameraPosition.xz);
    opacityFalloff = smoothstep(1.5f, 0.0f, distanceToCamera / maxFadeDistance);
    
    outColor *= opacityFalloff;
}