# https://stackoverflow.com/a/67044657/6692219
find_package(Vulkan COMPONENTS glslc)

file(
  GLOB shaders
  *.vert
  *.frag
  *.comp
  *.geom
  *.tesc
  *.tese
  *.mesh
  *.task
  *.rgen
  *.rchit
  *.rmiss
)

foreach(shader_path IN LISTS shaders)
  cmake_path(GET shader_path FILENAME shader)
  set(out_path ${resource_dir}/shaders/${shader}.spv)
  add_custom_command(
    OUTPUT ${out_path}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader_path} -o ${out_path}
    DEPENDS ${shader_path}
    COMMENT "Compiling ${shader}"
  )

  list(APPEND SPV_SHADERS ${out_path})
endforeach()

add_custom_target(shaders_compile ALL DEPENDS ${SPV_SHADERS})
