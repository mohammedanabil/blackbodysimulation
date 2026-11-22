#pragma once

#include <array>
#include <string>
#include <vector>

struct PinnedCurve {
    std::string name;
    double temperatureKelvin = 0.0;
    std::vector<double> wavelengths;
    std::vector<double> exitances;
    bool visible = true;
};

struct PhysicsQuantities {
    double peakWavelengthMeters = 0.0;
    double peakExitance = 0.0;
    double totalPower = 0.0;
    double uvFraction = 0.0;
    double visibleFraction = 0.0;
    double irFraction = 0.0;
};

struct AppState {
    double temperature = 5778.0;
    double constantEmissivity = 0.8;
    bool showRJ = false;
    bool showWien = false;
    bool logScale = false;
    bool showVisibleBand = true;
    bool showClassicalComparison = false;

    // Flexible wavelength range
    double minWavelengthNm = 100.0;
    double maxWavelengthNm = 3000.0;
    int wavelengthSamples = 1200;

    int selectedStar = 0;
    int selectedMaterial = 0;
    std::string emissivityError;
    std::array<char, 1024> excelFilePath{};
    std::vector<double> excelWavelengthsNanometers;
    double excelTemperatureKelvin = 3000.0;
    double excelEmissivity = 0.8;
    double activeExcelEmissivity = 0.8;
    bool excelWavelengthsLoaded = false;
    bool excelMode = false;
    std::string excelError;
    bool estimateTemperature = false;
    bool hasTemperatureEstimate = false;
    double estimatedTemperature = 0.0;
    double temperatureUncertainty = 0.0;
    double estimatedEmissivity = 1.0;
    double fitRmsResidual = 0.0;
    bool estimateSucceeded = false;

    // Physics metrics
    PhysicsQuantities physics{};

    // Multi-curve comparison
    std::vector<PinnedCurve> pinnedCurves;

    // Temperature Animation
    bool isAnimating = false;
    double animationSpeed = 500.0; // K per second
    double animMinT = 1000.0;
    double animMaxT = 10000.0;
    int animDirection = 1;
};
