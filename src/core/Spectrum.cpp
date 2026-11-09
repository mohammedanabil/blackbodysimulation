#include "Spectrum.h"

#include "EmissivityModel.h"
#include "PlanckModel.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

Spectrum SpectrumGenerator::generate(
    double temperatureKelvin,
    double minimumWavelengthMeters,
    double maximumWavelengthMeters,
    std::size_t steps)
{
    const IdealBlackbodyModel idealBlackbody;
    return generate(
        temperatureKelvin,
        minimumWavelengthMeters,
        maximumWavelengthMeters,
        steps,
        idealBlackbody);
}

Spectrum SpectrumGenerator::generate(
    double temperatureKelvin,
    double minimumWavelengthMeters,
    double maximumWavelengthMeters,
    std::size_t steps,
    const EmissivityModel& emissivityModel)
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
            emissivityModel.getEmissivity(
                wavelength * 1.0e9, temperatureKelvin) *
            PlanckModel::planck(wavelength, temperatureKelvin));
    }

    return spectrum;
}

Spectrum SpectrumGenerator::generateAtWavelengths(
    double temperatureKelvin,
    const std::vector<double>& wavelengthsNanometers,
    const EmissivityModel& emissivityModel)
{
    if (!std::isfinite(temperatureKelvin) || temperatureKelvin <= 0.0) {
        throw std::invalid_argument("temperature must be positive and finite");
    }
    if (wavelengthsNanometers.size() < 2) {
        throw std::invalid_argument(
            "at least two wavelength samples are required");
    }

    std::vector<double> sortedWavelengths = wavelengthsNanometers;
    for (double wavelength : sortedWavelengths) {
        if (!std::isfinite(wavelength) || wavelength <= 0.0) {
            throw std::invalid_argument(
                "wavelengths must be positive and finite");
        }
    }
    std::sort(sortedWavelengths.begin(), sortedWavelengths.end());
    if (std::adjacent_find(
            sortedWavelengths.begin(),
            sortedWavelengths.end()) != sortedWavelengths.end()) {
        throw std::invalid_argument("wavelengths must not contain duplicates");
    }

    Spectrum spectrum;
    spectrum.temperatureKelvin = temperatureKelvin;
    spectrum.minimumWavelengthMeters = sortedWavelengths.front() * 1.0e-9;
    spectrum.maximumWavelengthMeters = sortedWavelengths.back() * 1.0e-9;
    spectrum.wavelengths.reserve(sortedWavelengths.size());
    spectrum.exitances.reserve(sortedWavelengths.size());
    for (double wavelengthNm : sortedWavelengths) {
        const double wavelengthMeters = wavelengthNm * 1.0e-9;
        spectrum.wavelengths.push_back(wavelengthMeters);
        spectrum.exitances.push_back(
            emissivityModel.getEmissivity(wavelengthNm, temperatureKelvin) *
            PlanckModel::planck(wavelengthMeters, temperatureKelvin));
    }
    return spectrum;
}
