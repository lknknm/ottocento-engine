add_executable(ottocento-engine main.cxx)

target_link_libraries(ottocento-engine PUBLIC lib_ottocento_engine)

set_target_properties(
    ottocento-engine
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
