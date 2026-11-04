#include "core/ColorConverter.h"
#include "core/Spectrum.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

TEST_CASE("6500 K maps near white", "[color]")
{
    const Spectrum spectrum = SpectrumGenerator::generate(
        6500.0, 380.0e-9, 780.0e-9, 801);
    const RGB color = ColorConverter::spectrumToRGB(
        spectrum,
        std::filesystem::path(BLACKBODY_TEST_SOURCE_DIR) /
            "data/cie_1931.csv");

    REQUIRE(color.red > 0.75);
    REQUIRE(color.green > 0.75);
    REQUIRE(color.blue > 0.75);
}

TEST_CASE("3000 K maps orange", "[color]")
{
    const Spectrum spectrum = SpectrumGenerator::generate(
        3000.0, 380.0e-9, 780.0e-9, 801);
    const RGB color = ColorConverter::spectrumToRGB(
        spectrum,
        std::filesystem::path(BLACKBODY_TEST_SOURCE_DIR) /
            "data/cie_1931.csv");

    REQUIRE(color.red > color.green);
    REQUIRE(color.green > color.blue);
}
