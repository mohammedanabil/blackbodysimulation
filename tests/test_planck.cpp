#include "core/PlanckModel.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/Spectrum.h"
#include "core/TemperatureEstimator.h"

TEST_CASE("The Sun's peak wavelength is near 502 nm", "[planck]")
{
    constexpr double sunTemperatureKelvin = 5778.0;
    const double peakMeters = PlanckModel::wienPeak(sunTemperatureKelvin);
    const double peakNanometers = peakMeters * 1.0e9;

    REQUIRE(peakNanometers == Catch::Approx(502.0).margin(2.0));
}

TEST_CASE("Planck fit recovers a known temperature", "[spectroradiometry]")
{
    const Spectrum spectrum = SpectrumGenerator::generate(
        1500.0,
        100.0e-9,
        3000.0e-9,
        512);

    const TemperatureEstimate estimate =
        TemperatureEstimator::planckFit(spectrum);

    REQUIRE(estimate.status == TemperatureFitStatus::Ok);
    REQUIRE(estimate.temperatureKelvin ==
            Catch::Approx(1500.0).margin(15.0));
    REQUIRE(estimate.emissivity == Catch::Approx(1.0).margin(0.01));
}

TEST_CASE("Wien estimator returns a temperature from the spectral peak", "[spectroradiometry]")
{
    const Spectrum spectrum = SpectrumGenerator::generate(
        3000.0,
        100.0e-9,
        3000.0e-9,
        512);

    const TemperatureEstimate estimate =
        TemperatureEstimator::wien(spectrum);

    REQUIRE(estimate.status == TemperatureFitStatus::Ok);
    REQUIRE(estimate.temperatureKelvin ==
            Catch::Approx(3000.0).epsilon(0.03));
}
