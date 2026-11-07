#include "core/ColorConverter.h"
#include "core/Spectrum.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
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

TEST_CASE("Spectral exitance scaling preserves swatch colors", "[color]")
{
    const std::filesystem::path ciePath =
        std::filesystem::path(BLACKBODY_TEST_SOURCE_DIR) /
        "data/cie_1931.csv";

    for (double temperature : {3000.0, 5778.0, 10000.0}) {
        const Spectrum spectrum = SpectrumGenerator::generate(
            temperature, 380.0e-9, 780.0e-9, 801);
        const RGB color = ColorConverter::spectrumToRGB(spectrum, ciePath);

        REQUIRE(std::isfinite(color.red));
        REQUIRE(std::isfinite(color.green));
        REQUIRE(std::isfinite(color.blue));
        REQUIRE(color.red >= 0.0);
        REQUIRE(color.green >= 0.0);
        REQUIRE(color.blue >= 0.0);
        REQUIRE(color.red <= 1.0);
        REQUIRE(color.green <= 1.0);
        REQUIRE(color.blue <= 1.0);
    }
}
