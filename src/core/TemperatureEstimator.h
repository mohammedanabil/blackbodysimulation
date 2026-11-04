#pragma once

#include "core/Spectrum.h"

#include <string>

enum class TemperatureFitStatus {
    Ok,
    LowSignal,
    NoConvergence,
    InvalidInput,
    OutOfRange
};

struct TemperatureEstimate {
    double temperatureKelvin = 0.0;
    double uncertaintyKelvin = 0.0;
    double emissivity = 1.0;
    double rmsResidual = 0.0;
    int iterations = 0;
    TemperatureFitStatus status = TemperatureFitStatus::InvalidInput;
    std::string method;
};

class TemperatureEstimator {
public:
    static TemperatureEstimate wien(const Spectrum& spectrum);
    static TemperatureEstimate planckFit(
        const Spectrum& spectrum,
        double minimumTemperatureKelvin = 200.0,
        double maximumTemperatureKelvin = 20000.0,
        bool fitEmissivity = true);
};
