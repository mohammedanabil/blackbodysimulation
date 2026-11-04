#pragma once

class PlanckModel {
public:
    static double planck(double wavelengthMeters, double temperatureKelvin);
    static double rayleighJeans(double wavelengthMeters, double temperatureKelvin);
    static double wienPeak(double temperatureKelvin);
    static double totalPower(double temperatureKelvin);
};
