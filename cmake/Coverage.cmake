function(enable_coverage)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
		# Coverage
		option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

		if(ENABLE_COVERAGE)
			# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
				add_compile_options("$<$<CONFIG:DEBUG>:--coverage>")
				add_link_options("$<$<CONFIG:DEBUG>:--coverage>")
			# else()
				# add_compile_options("$<$<CONFIG:DEBUG>:-fprofile-instr-generate;-fcoverage-mapping>")

				# add_link_options("$<$<CONFIG:DEBUG>:-fprofile-instr-generate;-fcoverage-mapping>")
			# endif()
		endif()
	endif()
endfunction()
