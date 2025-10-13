# https://stackoverflow.com/a/67044657/6692219
find_package(Vulkan COMPONENTS glslc)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(
    GLOB shaders
    ${SHADER_DIR}/*.vert
    ${SHADER_DIR}/*.frag
    ${SHADER_DIR}/*.comp
    ${SHADER_DIR}/*.geom
    ${SHADER_DIR}/*.tesc
    ${SHADER_DIR}/*.tese
    ${SHADER_DIR}/*.mesh
    ${SHADER_DIR}/*.task
    ${SHADER_DIR}/*.rgen
    ${SHADER_DIR}/*.rchit
    ${SHADER_DIR}/*.rmiss
)

foreach(shader_path IN LISTS shaders)
    cmake_path(GET shader_path FILENAME shader)
    set(out_path ${CMAKE_BINARY_DIR}/shaders/${shader}.spv)
    add_custom_command(
        OUTPUT ${out_path}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader_path} -o ${out_path}
        DEPENDS ${shader_path}
        COMMENT "Compiling ${shader}"
    )

    list(APPEND SPV_SHADERS ${out_path})
endforeach()

add_custom_target(shaders_compile ALL DEPENDS ${SPV_SHADERS})
