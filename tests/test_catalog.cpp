#include "data/StarCatalog.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE("Star catalog contains representative presets", "[data]")
{
    const auto& stars = StarCatalog::stars();

    REQUIRE(stars.size() >= 5);
    REQUIRE(StarCatalog::star(0).name == "Sun");
    REQUIRE(StarCatalog::star(0).temperatureKelvin == 5778.0);

    bool foundCMB = false;
    for (const auto& preset : stars) {
        if (preset.name.find("CMB") != std::string_view::npos) {
            foundCMB = true;
            REQUIRE(preset.temperatureKelvin == Catch::Approx(2.725));
        }
    }
    REQUIRE(foundCMB);
}
