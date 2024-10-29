# From neovim project

# Set default build type to Debug. Also limit the list of allowable build types
# to the ones defined in variable allowableBuildTypes.
#
# The correct way to specify build type (for example Release) for
# single-configuration generators (Make and Ninja) is to run
#
# cmake -B build -D CMAKE_BUILD_TYPE=Release
# cmake --build build
#
# while for multi-configuration generators (Visual Studio, Xcode and Ninja
# Multi-Config) is to run
#
# cmake -B build
# cmake --build build --config Release
#
# Passing CMAKE_BUILD_TYPE for multi-config generators will now not only
# not be used, but also generate a warning for the user.
function(set_default_buildtype)
	set(allowableBuildTypes Debug Release MinSizeRel RelWithDebInfo)

	get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(isMultiConfig)
		set(CMAKE_CONFIGURATION_TYPES ${allowableBuildTypes} PARENT_SCOPE)
		if(CMAKE_BUILD_TYPE)
			message(WARNING "CMAKE_BUILD_TYPE specified which is ignored on \
			multi-configuration generators. Defaulting to Debug build type.")
		endif()
	else()
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowableBuildTypes}")
		if(NOT CMAKE_BUILD_TYPE)
			message(STATUS "CMAKE_BUILD_TYPE not specified, default is 'Debug'")
			set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
		elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
			message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
		else()
			message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
		endif()
	endif()
endfunction()
