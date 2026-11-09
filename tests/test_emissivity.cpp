#include "core/EmissivityModel.h"
#include "core/PlanckModel.h"
#include "core/Spectrum.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {

std::filesystem::path materialPath(const char* filename)
{
    return std::filesystem::path(BLACKBODY_TEST_SOURCE_DIR) /
           "data/materials" / filename;
}

} // namespace

TEST_CASE("Ideal and constant emissivity models return valid values", "[emissivity]")
{
    const IdealBlackbodyModel ideal;
    REQUIRE(ideal.getEmissivity(500.0, 3000.0) == 1.0);

    const ConstantEmissivityModel gray(0.35);
    REQUIRE(gray.getEmissivity(500.0, 3000.0) == Catch::Approx(0.35));
    REQUIRE(ConstantEmissivityModel(2.0).getEmissivity(500.0, 3000.0) == 1.0);
    REQUIRE(ConstantEmissivityModel(-1.0).getEmissivity(500.0, 3000.0) == 0.0);
}

TEST_CASE("Tabulated emissivity interpolates and clamps endpoints", "[emissivity]")
{
    const TabulatedEmissivityModel steel =
        TabulatedEmissivityModel::loadFromCSV(
            materialPath("oxidized_steel.csv"));

    REQUIRE(steel.getEmissivity(50.0, 3000.0) == Catch::Approx(0.72));
    REQUIRE(steel.getEmissivity(400.0, 3000.0) == Catch::Approx(0.655));
    REQUIRE(steel.getEmissivity(4000.0, 3000.0) == Catch::Approx(0.35));
}

TEST_CASE("Spectrum generation applies the emissivity strategy", "[emissivity]")
{
    const ConstantEmissivityModel gray(0.25);
    const Spectrum spectrum = SpectrumGenerator::generate(
        3000.0, 500.0e-9, 500.0e-9 + 1.0e-9, 2, gray);

    REQUIRE(spectrum.exitances[0] ==
            Catch::Approx(0.25 * PlanckModel::planck(500.0e-9, 3000.0)));
}
