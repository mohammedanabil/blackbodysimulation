#include "data/StarCatalog.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Star catalog contains representative presets", "[data]")
{
    const auto& stars = StarCatalog::stars();

    REQUIRE(stars.size() >= 3);
    REQUIRE(StarCatalog::star(0).name == "Sun");
    REQUIRE(StarCatalog::star(0).temperatureKelvin == 5778.0);
}
