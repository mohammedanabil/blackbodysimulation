#include "TemperatureEstimator.h"

#include "Constants.h"
#include "PlanckModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace {

bool validSpectrum(const Spectrum& spectrum)
{
    const std::size_t size = spectrum.wavelengths.size();
    if (size < 2 || size != spectrum.intensities.size()) {
        return false;
    }

    for (std::size_t i = 0; i < size; ++i) {
        if (!std::isfinite(spectrum.wavelengths[i]) ||
            !std::isfinite(spectrum.intensities[i]) ||
            spectrum.wavelengths[i] <= 0.0) {
            return false;
        }
    }
    return true;
}

double findScale(const std::vector<double>& measured,
                 const std::vector<double>& model,
                 bool fitEmissivity,
                 double& emissivity)
{
    double numerator = 0.0;
    double denominator = 0.0;
    for (std::size_t i = 0; i < measured.size(); ++i) {
        numerator += measured[i] * model[i];
        denominator += model[i] * model[i];
    }

    emissivity = fitEmissivity && denominator > 0.0
        ? std::clamp(numerator / denominator, 0.01, 1.0)
        : 1.0;
    return emissivity;
}

} // namespace

TemperatureEstimate TemperatureEstimator::wien(const Spectrum& spectrum)
{
    TemperatureEstimate result;
    result.method = "wien";
    if (!validSpectrum(spectrum)) {
        return result;
    }

    std::size_t peakIndex = 0;
    double peakIntensity = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < spectrum.intensities.size(); ++i) {
        if (spectrum.intensities[i] > peakIntensity) {
            peakIntensity = spectrum.intensities[i];
            peakIndex = i;
        }
    }

    result.temperatureKelvin =
        Constants::wien / spectrum.wavelengths[peakIndex];
    if (!std::isfinite(result.temperatureKelvin) || result.temperatureKelvin < 200.0 ||
        result.temperatureKelvin > 20000.0) {
        result.status = TemperatureFitStatus::OutOfRange;
    } else {
        result.status = TemperatureFitStatus::Ok;
    }
    return result;
}

TemperatureEstimate TemperatureEstimator::planckFit(
    const Spectrum& spectrum,
    double minimumTemperatureKelvin,
    double maximumTemperatureKelvin,
    bool fitEmissivity)
{
    TemperatureEstimate result;
    result.method = "planck_fit";
    if (!validSpectrum(spectrum) ||
        !std::isfinite(minimumTemperatureKelvin) ||
        !std::isfinite(maximumTemperatureKelvin) ||
        minimumTemperatureKelvin <= 0.0 ||
        maximumTemperatureKelvin <= minimumTemperatureKelvin) {
        return result;
    }

    double signalPeak = 0.0;
    for (double value : spectrum.intensities) {
        signalPeak = std::max(signalPeak, std::abs(value));
    }
    if (signalPeak <= 0.0) {
        result.status = TemperatureFitStatus::LowSignal;
        return result;
    }

    const double minLog =
        std::log(std::max(100.0, minimumTemperatureKelvin));
    const double maxLog =
        std::log(maximumTemperatureKelvin);

    const auto objective = [&](double logTemperature) {
        const double temperature = std::exp(logTemperature);
        std::vector<double> model;
        model.reserve(spectrum.wavelengths.size());
        for (double wavelength : spectrum.wavelengths) {
            model.push_back(PlanckModel::planck(wavelength, temperature));
        }

        double emissivity = 1.0;
        findScale(spectrum.intensities, model, fitEmissivity, emissivity);
        double residualSum = 0.0;
        for (std::size_t i = 0; i < model.size(); ++i) {
            const double residual =
                emissivity * model[i] - spectrum.intensities[i];
            residualSum += residual * residual;
        }
        return residualSum;
    };

    TemperatureEstimate wienResult = wien(spectrum);
    double center = std::log(std::clamp(
        wienResult.temperatureKelvin, minimumTemperatureKelvin,
        maximumTemperatureKelvin));
    double bestLog = center;
    double bestCost = objective(bestLog);

    constexpr double goldenRatio = 0.6180339887498949;
    double left = minLog;
    double right = maxLog;
    double x1 = right - goldenRatio * (right - left);
    double x2 = left + goldenRatio * (right - left);
    double f1 = objective(x1);
    double f2 = objective(x2);
    for (int iteration = 0; iteration < 100; ++iteration) {
        result.iterations = iteration + 1;
        if (f1 < f2) {
            right = x2;
            x2 = x1;
            f2 = f1;
            x1 = right - goldenRatio * (right - left);
            f1 = objective(x1);
        } else {
            left = x1;
            x1 = x2;
            f1 = f2;
            x2 = left + goldenRatio * (right - left);
            f2 = objective(x2);
        }
        if (std::abs(right - left) < 1.0e-8) {
            break;
        }
    }
    bestLog = 0.5 * (left + right);
    bestCost = objective(bestLog);

    result.temperatureKelvin = std::exp(bestLog);
    std::vector<double> model;
    model.reserve(spectrum.wavelengths.size());
    for (double wavelength : spectrum.wavelengths) {
        model.push_back(PlanckModel::planck(wavelength, result.temperatureKelvin));
    }
    findScale(spectrum.intensities, model, fitEmissivity, result.emissivity);

    result.rmsResidual = std::sqrt(
        std::max(0.0, bestCost) / static_cast<double>(model.size()));

    const double h = 1.0e-4;
    const double costMinus = objective(bestLog - h);
    const double costPlus = objective(bestLog + h);
    const double curvature =
        std::max((costPlus - 2.0 * bestCost + costMinus) / (h * h), 1.0e-12);
    const double variance =
        bestCost / static_cast<double>(std::max<std::size_t>(1, model.size() - 2));
    const double sigmaLogT = std::sqrt(variance / curvature);
    result.uncertaintyKelvin =
        sigmaLogT * result.temperatureKelvin;

    result.status = std::isfinite(result.temperatureKelvin) &&
                            result.temperatureKelvin >= minimumTemperatureKelvin &&
                            result.temperatureKelvin <= maximumTemperatureKelvin
        ? TemperatureFitStatus::Ok
        : TemperatureFitStatus::NoConvergence;
    return result;
}
