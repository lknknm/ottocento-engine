# https://stackoverflow.com/a/67044657/6692219
find_package(Vulkan)
find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(GLOB shaders
	${SHADER_DIR}/*.vert
	${SHADER_DIR}/*.frag
	# ${SHADER_DIR}/*.comp
	# ${SHADER_DIR}/*.geom
	# ${SHADER_DIR}/*.tesc
	# ${SHADER_DIR}/*.tese
	# ${SHADER_DIR}/*.mesh
	# ${SHADER_DIR}/*.task
	# ${SHADER_DIR}/*.rgen
	# ${SHADER_DIR}/*.rchit
	# ${SHADER_DIR}/*.rmiss
)

foreach(shader IN LISTS shaders)
	cmake_path(GET shader EXTENSION ext)
	cmake_path(GET shader STEM basename)
	if(ext STREQUAL ".vert")
		set(spv_base "${basename}.vert")
	elseif(ext STREQUAL ".frag")
		set(spv_base "${basename}.frag")
	else()
		set(spv_base "${basename}")
	endif()

	add_custom_command(OUTPUT ${SHADER_DIR}/${spv_base}.spv
		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader} -o ${SHADER_DIR}/${spv_base}.spv
		DEPENDS ${shader}
		COMMENT "Compiling ${spv_base}")

	list(APPEND SPV_SHADERS ${SHADER_DIR}/${spv_base}.spv)
endForeach()

add_custom_target(shaders_compile ALL DEPENDS ${SPV_SHADERS})
