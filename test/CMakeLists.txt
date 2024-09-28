
find_package(Catch2 REQUIRED)

include(Catch)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR} *.cxx)

add_executable(ottocento-test-suite ${sources})
target_link_libraries(ottocento-test-suite PRIVATE
   lib_ottocento_engine
   Catch2::Catch2WithMain)

   
catch_discover_tests(ottocento-test-suite)