#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

TEST_CASE("First test") {
  fmt::print("First test :D\n");
  REQUIRE(true == true);
}