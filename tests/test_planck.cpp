#include "core/PlanckModel.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/Spectrum.h"
#include "core/TemperatureEstimator.h"

#include <cmath>

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

TEST_CASE("Planck exitance equals the M formula with pi", "[planck]")
{
    constexpr double h = 6.62607015e-34;
    constexpr double c = 299792458.0;
    constexpr double k = 1.380649e-23;
    constexpr double pi = 3.14159265358979323846;

    const double samples[][2] = {
        {500.0e-9, 5778.0},
        {966.0e-9, 3000.0},
        {289.8e-9, 10000.0},
        {10.0e-6, 300.0}
    };

    for (const auto& sample : samples) {
        const double wavelength = sample[0];
        const double temperature = sample[1];
        const double x = h * c / (wavelength * k * temperature);
        const double expected =
            2.0 * pi * h * c * c /
            (std::pow(wavelength, 5) * std::expm1(x));
        REQUIRE(PlanckModel::planck(wavelength, temperature) ==
                Catch::Approx(expected).epsilon(1.0e-9));
    }
}

TEST_CASE("Solar spectral exitance peak and Wien law", "[planck][spectroradiometry]")
{
    const Spectrum spectrum = SpectrumGenerator::generate(
        5778.0, 100.0e-9, 3000.0e-9, 1200);

    std::size_t peakIndex = 0;
    for (std::size_t index = 1; index < spectrum.exitances.size(); ++index) {
        if (spectrum.exitances[index] > spectrum.exitances[peakIndex]) {
            peakIndex = index;
        }
    }

    const double peakNm = spectrum.wavelengths[peakIndex] * 1.0e9;
    REQUIRE(peakNm == Catch::Approx(501.5).margin(3.0));
    REQUIRE(spectrum.exitances[peakIndex] ==
            Catch::Approx(8.29e13).epsilon(0.03));
    REQUIRE(spectrum.wavelengths[peakIndex] * 5778.0 ==
            Catch::Approx(2.898e-3).epsilon(0.005));
}

TEST_CASE("Rayleigh-Jeans converges and Planck stays finite at short wavelengths", "[planck]")
{
    constexpr double temperature = 3000.0;
    const double wavelength = 1.0e-2;
    const double ratio =
        PlanckModel::rayleighJeans(wavelength, temperature) /
        PlanckModel::planck(wavelength, temperature);
    REQUIRE(ratio == Catch::Approx(1.0).epsilon(1.0e-3));

    const double shortWavelengthExitance =
        PlanckModel::planck(1.0e-12, temperature);
    REQUIRE(std::isfinite(shortWavelengthExitance));
    REQUIRE(shortWavelengthExitance == 0.0);
}
