#pragma once

struct AppState {
    double temperature = 5778.0;
    bool showRJ = false;
    bool logScale = false;
    int selectedStar = 0;
    bool estimateTemperature = false;
    bool hasTemperatureEstimate = false;
    double estimatedTemperature = 0.0;
    double temperatureUncertainty = 0.0;
    double estimatedEmissivity = 1.0;
    double fitRmsResidual = 0.0;
    bool estimateSucceeded = false;
};
