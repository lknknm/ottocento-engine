#version 450

const float gridSize     = 2.0f;
const float cellSize     = 1.0f;
const float halfCellSize = cellSize * 0.5f;

const float subcellSize     = 0.1f;
const float halfSubcellSize = subcellSize * 0.5f;


vec3 gridPlane[6] = vec3[](
vec3(-1, -1, 0),
vec3(1, -1, 0),
vec3(1, 1, 0),
vec3(-1, -1, 0),
vec3(-1, 1, 0),
vec3(1, 1, 0)
);


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 fragColor;

vec3 cellCoords      = mod(gridPlane[gl_VertexIndex] + halfCellSize, cellSize);
vec3 subcellCoords   = mod(gridPlane[gl_VertexIndex] + halfSubcellSize, subcellSize);

// normal vertice projection
void main() {
    gl_Position = ubo.proj * ubo.view * vec4(gridPlane[gl_VertexIndex].xyz, 1.0);
    fragColor = vec3(normalize(cellCoords.x), normalize(cellCoords.y), 0.0f);
}