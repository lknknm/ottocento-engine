function(enable_sanitizers)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")

		set(SANITIZERS "")

		# UBSAN
		option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" TRUE)

		if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
			message(STATUS "${CMAKE_PROJECT_NAME}: Enabling undefined behavior sanitizer")
			list(APPEND SANITIZERS "undefined")
		endif()

		set(extra_sanitizers none address thread leak)
		if(NOT APPLE)
			list(APPEND extra_sanitizers memory)
		endif()

		if(NOT EXTRA_SANITIZER)
			set(EXTRA_SANITIZER "address" CACHE STRING "Sanitizer other than UBSAN")
		endif()

		set_property(CACHE EXTRA_SANITIZER PROPERTY STRINGS ${extra_sanitizers})

		if(EXTRA_SANITIZER AND NOT "${EXTRA_SANITIZER}" STREQUAL "none")
			list(APPEND SANITIZERS ${EXTRA_SANITIZER})
			message(STATUS "${CMAKE_PROJECT_NAME}: Enabling ${EXTRA_SANITIZER} sanitizer")
		endif()

		list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)
	endif()

	if(LIST_OF_SANITIZERS)
		if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
			add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=${LIST_OF_SANITIZERS}>")
			add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=${LIST_OF_SANITIZERS}>")

			if(WIN32)
				set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
			endif()
		endif()
	endif()
endfunction()
