#pragma once

#include <cstddef>

class PlanckModel {
public:
    static double planck(double wavelengthMeters, double temperatureKelvin);
    static double rayleighJeans(double wavelengthMeters, double temperatureKelvin);
    static double wienApproximation(double wavelengthMeters, double temperatureKelvin);
    static double wienPeak(double temperatureKelvin);
    static double totalPower(double temperatureKelvin);
    static double integrateBandPower(
        double minWavelengthMeters,
        double maxWavelengthMeters,
        double temperatureKelvin,
        std::size_t intervals = 200);
};
