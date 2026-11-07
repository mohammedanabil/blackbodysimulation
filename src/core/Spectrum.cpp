#include "Spectrum.h"

#include "PlanckModel.h"

#include <cmath>
#include <stdexcept>
#include <vector>

Spectrum SpectrumGenerator::generate(
    double temperatureKelvin,
    double minimumWavelengthMeters,
    double maximumWavelengthMeters,
    std::size_t steps)
{
    if (!std::isfinite(temperatureKelvin) || temperatureKelvin <= 0.0) {
        throw std::invalid_argument("temperature must be positive and finite");
    }
    if (!std::isfinite(minimumWavelengthMeters) || minimumWavelengthMeters <= 0.0) {
        throw std::invalid_argument("minimum wavelength must be positive and finite");
    }
    if (!std::isfinite(maximumWavelengthMeters) || maximumWavelengthMeters <= minimumWavelengthMeters) {
        throw std::invalid_argument(
            "maximum wavelength must be finite and greater than minimum wavelength");
    }
    if (steps < 2) {
        throw std::invalid_argument("steps must be at least 2");
    }

    Spectrum spectrum{
        temperatureKelvin,
        minimumWavelengthMeters,
        maximumWavelengthMeters,
        {},
        {}
    };
    spectrum.wavelengths.reserve(steps);
    spectrum.exitances.reserve(steps);

    const double interval = maximumWavelengthMeters - minimumWavelengthMeters;
    for (std::size_t index = 0; index < steps; ++index) {
        const double fraction =
            static_cast<double>(index) / static_cast<double>(steps - 1);
        const double wavelength = minimumWavelengthMeters + fraction * interval;
        spectrum.wavelengths.push_back(wavelength);
        spectrum.exitances.push_back(
            PlanckModel::planck(wavelength, temperatureKelvin));
    }

    return spectrum;
}
