function(enable_sanitizers)

	set(SANITIZERS "")
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")

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

	# Windows
	elseif(WIN32)

		# ASAN
		option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" TRUE)
		if(ENABLE_SANITIZER_ADDRESS)
			message(STATUS "${CMAKE_PROJECT_NAME}: Enabling address sanitizer")
			list(APPEND SANITIZERS "address")
		endif()

		# FUZZER
		option(ENABLE_SANITIZER_FUZZER "Enable fuzzer sanitizer" TRUE)
		if(ENABLE_SANITIZER_FUZZER)
			message(STATUS "${CMAKE_PROJECT_NAME}: Enabling fuzzer sanitizer")
			list(APPEND SANITIZERS "fuzzer")
		endif()
	endif()

	if(LIST_OF_SANITIZERS)
		if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
			if(WIN32)
				set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
				add_compile_options("$<$<CONFIG:DEBUG>:/fsanitize=${LIST_OF_SANITIZERS}>")
			else()

				add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=${LIST_OF_SANITIZERS}>")
				add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=${LIST_OF_SANITIZERS}>")
			endif()
		endif()
	endif()
endfunction()
