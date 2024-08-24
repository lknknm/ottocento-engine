#version 450

layout(location = 0) in vec2 fragCoords;
layout(location = 0) out vec4 outColor;

const float gridSize                = 100.0f;

const float cellSize                = 1.0f;
const float halfcellSize            = cellSize * 0.5f;
const float subcellSize             = 0.1f;
const float halfsubcellSize         = subcellSize * 0.5f;

const float minFadeDistance = gridSize * 0.05f;
const float maxFadeDistance = 25.0f;
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

void main() {
    float dx = fwidth(cellCoords.x);
    float dy = fwidth(cellCoords.y);
    
    vec2 adjustedCellLineThickness    = vec2(0.5f * (cellLineThickness.x + dx), 0.5f * (cellLineThickness.y + dy));
    vec2 adjustedSubcellLineThickness = vec2(0.5f * (subcellLineThickness.x + dx), 0.5f * (subcellLineThickness.y + dy));
    
    float distanceToCamera = length(cellCoords - cameraPosition.xz);
    opacityFalloff = smoothstep(1.0f, 0.0f, distanceToCamera / maxFadeDistance);
    
    outColor = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    if (distanceToSubcell.x < adjustedSubcellLineThickness.x * 0.5 || distanceToSubcell.y < adjustedSubcellLineThickness.y * 0.5 )
        outColor = subcellColor;
    if (distanceToCell.x < adjustedCellLineThickness.x * 0.5 || distanceToCell.y < adjustedCellLineThickness.y * 0.5 )
        outColor = cellColor;
    if (fragCoords.y > -adjustedCellLineThickness.y * 0.5 && fragCoords.y < adjustedCellLineThickness.y * 0.5 )
        outColor = vec4(0.0f, 1.0f, 0.0f, 0.25f);
    if (fragCoords.x > -adjustedCellLineThickness.x * 0.5 && fragCoords.x < adjustedCellLineThickness.x * 0.5 )
        outColor = vec4(1.0f, 0.0f, 0.0f, 0.25f);
}