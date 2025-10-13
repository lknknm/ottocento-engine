file(GLOB textures *.png)

foreach(texture_path IN LISTS textures)
  cmake_path(GET texture_path FILENAME texture)
  set(out_path ${resource_dir}/textures/${texture})
  add_custom_command(
    OUTPUT ${out_path}
    COMMAND ${CMAKE_COMMAND} -E copy "${texture_path}" "${out_path}"
    DEPENDS ${texture_path}
    COMMENT "Copying ${texture}"
  )
  list(APPEND TEXTURES ${out_path})
endforeach()

add_custom_target(copy_textures ALL DEPENDS ${TEXTURES})
