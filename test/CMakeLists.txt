
find_package(Catch2 REQUIRED)
find_package(fmt CONFIG REQUIRED)

include(Catch)

file(GLOB sources *.cxx)

add_executable(ottocento-test-suite ${sources})
target_link_libraries(ottocento-test-suite PRIVATE
   lib_ottocento_engine
   Catch2::Catch2WithMain
   fmt::fmt)

   
catch_discover_tests(ottocento-test-suite)