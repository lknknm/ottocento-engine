file(GLOB models *.obj *.mtl *.png)

foreach(model_path IN LISTS models)
  cmake_path(GET model_path FILENAME model)
  set(out_path ${resource_dir}/models/${model})
  add_custom_command(
    OUTPUT ${out_path}
    COMMAND ${CMAKE_COMMAND} -E copy "${model_path}" "${out_path}"
    DEPENDS ${model_path}
    COMMENT "Copying ${model}"
  )
  list(APPEND MODELS ${out_path})
endforeach()

add_custom_target(copy_models ALL DEPENDS ${MODELS})
