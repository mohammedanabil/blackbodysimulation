#pragma once

#include <array>
#include <string>
#include <vector>

struct AppState {
    double temperature = 5778.0;
    double constantEmissivity = 0.8;
    bool showRJ = false;
    bool logScale = false;
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
};
