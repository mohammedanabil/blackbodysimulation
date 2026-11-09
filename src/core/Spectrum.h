#pragma once

#include <cstddef>
#include <vector>

class EmissivityModel;

struct Spectrum {
    double temperatureKelvin = 0.0;
    double minimumWavelengthMeters = 0.0;
    double maximumWavelengthMeters = 0.0;
    std::vector<double> wavelengths;
    std::vector<double> exitances;
};

class SpectrumGenerator {
public:
    static Spectrum generate(
        double temperatureKelvin,
        double minimumWavelengthMeters,
        double maximumWavelengthMeters,
        std::size_t steps);
    static Spectrum generate(
        double temperatureKelvin,
        double minimumWavelengthMeters,
        double maximumWavelengthMeters,
        std::size_t steps,
        const EmissivityModel& emissivityModel);
};
