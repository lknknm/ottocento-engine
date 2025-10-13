file(GLOB matcaps *.png)

foreach(matcap_path IN LISTS matcaps)
  cmake_path(GET matcap_path FILENAME matcap)
  set(out_path ${resource_dir}/matcap/${matcap})
  add_custom_command(
    OUTPUT ${out_path}
    COMMAND ${CMAKE_COMMAND} -E copy "${matcap_path}" "${out_path}"
    DEPENDS ${matcap_path}
    COMMENT "Copying ${matcap}"
  )
  list(APPEND MATCAPS ${out_path})
endforeach()

add_custom_target(copy_matcaps ALL DEPENDS ${MATCAPS})
