# From here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

function(enable_warnings)
	option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" FALSE)
	option(WARNINGS_DISABLE "Disable warnings" FALSE)
	option(WARNING_EFFC++ "Enable warnings for effective c++" TRUE)

	set(MSVC_WARNINGS
		/W4          # Baseline reasonable warnings
		/w14242      # 'identifier': conversion from 'type1' to 'type1', possible loss of data
		/w14254      # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
		/w14263      # 'function': member function does not override any base class virtual member function
		/w14265      # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
		/w14287      # 'operator': unsigned/negative constant mismatch
		/we4289      # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
		/w14296      # 'operator': expression is always 'boolean_value'
		/w14311      # 'variable': pointer truncation from 'type1' to 'type2'
		/w14545      # expression before comma evaluates to a function which is missing an argument list
		/w14546      # function call before comma missing argument list
		/w14547      # 'operator': operator before comma has no effect; expected operator with side-effect
		/w14549      # 'operator': operator before comma has no effect; did you intend 'operator'?
		/w14555      # expression has no effect; expected expression with side- effect
		/w14619      # pragma warning: there is no warning number 'number'
		/w14640      # Enable warning on thread un-safe static member initialization
		/w14826      # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
		/w14905      # wide string literal cast to 'LPSTR'
		/w14906      # string literal cast to 'LPWSTR'
		/w14928      # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
		/wd4996      # disable 'fopen': This function or variable may be unsafe.
		/permissive- # standards conformance mode for MSVC compiler.
	)

	set(CLANG_WARNINGS
		-Wall
		-Wextra
		-Wshadow
		-pedantic
		-Wnon-virtual-dtor         # Warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors.
		-Wold-style-cast           # warn for c-style casts
		-Wcast-align               # warn for potential performance problem casts
		-Wunused                   # warn on anything being unused
		-Woverloaded-virtual       # warn if you overload (not override) a virtual function
		-Wpedantic                 # warn if non-standard C++ is used
		-Wconversion               # warn on type conversions that may lose data
		-Wsign-conversion          # warn on sign conversions
		-Wnull-dereference         # warn if a null dereference is detected
		-Wdouble-promotion         # warn if float is implicit promoted to double
		-Wformat=2                 # warn on security issues around functions that format output (ie printf)
		-Wno-unused-lambda-capture # We like explicit capture
	)

	if(WARNINGS_AS_ERRORS)
		set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
		set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
	endif()

	if(WARNINGS_DISABLE)
		set(CLANG_WARNINGS -w)
		set(MSVC_WARNINGS /w)
	endif()

	if(WIN32)
		set(PROJECT_WARNINGS ${MSVC_WARNINGS})
	else()
		set(PROJECT_WARNINGS ${CLANG_WARNINGS})
	endif()

	add_compile_options(${PROJECT_WARNINGS})
endfunction()
