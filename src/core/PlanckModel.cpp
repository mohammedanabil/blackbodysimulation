#include "PlanckModel.h"

#include "Constants.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

void validatePositive(double value, const char* name)
{
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(name);
    }
}

} // namespace

double PlanckModel::planck(double wavelengthMeters, double temperatureKelvin)
{
    validatePositive(wavelengthMeters, "wavelength must be positive and finite");
    validatePositive(temperatureKelvin, "temperature must be positive and finite");

    const double exponent = (Constants::planck * Constants::speedOfLight) /
                            (wavelengthMeters * Constants::boltzmann * temperatureKelvin);

    if (exponent > 700.0) {
        return 0.0;
    }
    if (!std::isfinite(exponent)) {
        return 0.0;
    }

    const double wavelengthToFifth = std::pow(wavelengthMeters, 5);
    const double denominator = std::expm1(exponent);

    return 2.0 * Constants::pi * Constants::planck *
           Constants::speedOfLight * Constants::speedOfLight /
           (wavelengthToFifth * denominator);
}

double PlanckModel::rayleighJeans(double wavelengthMeters, double temperatureKelvin)
{
    validatePositive(wavelengthMeters, "wavelength must be positive and finite");
    validatePositive(temperatureKelvin, "temperature must be positive and finite");

    return 2.0 * Constants::pi * Constants::speedOfLight *
           Constants::boltzmann * temperatureKelvin /
           std::pow(wavelengthMeters, 4);
}

double PlanckModel::wienApproximation(double wavelengthMeters, double temperatureKelvin)
{
    validatePositive(wavelengthMeters, "wavelength must be positive and finite");
    validatePositive(temperatureKelvin, "temperature must be positive and finite");

    const double exponent = (Constants::planck * Constants::speedOfLight) /
                            (wavelengthMeters * Constants::boltzmann * temperatureKelvin);

    if (exponent > 700.0) {
        return 0.0;
    }
    if (!std::isfinite(exponent)) {
        return 0.0;
    }

    const double wavelengthToFifth = std::pow(wavelengthMeters, 5);
    const double denominator = std::exp(exponent);

    return 2.0 * Constants::pi * Constants::planck *
           Constants::speedOfLight * Constants::speedOfLight /
           (wavelengthToFifth * denominator);
}

double PlanckModel::wienPeak(double temperatureKelvin)
{
    validatePositive(temperatureKelvin, "temperature must be positive and finite");
    return Constants::wien / temperatureKelvin;
}

double PlanckModel::totalPower(double temperatureKelvin)
{
    validatePositive(temperatureKelvin, "temperature must be positive and finite");
    const double temperatureSquared = temperatureKelvin * temperatureKelvin;
    return Constants::stefanBoltzmann * temperatureSquared * temperatureSquared;
}

double PlanckModel::integrateBandPower(
    double minWavelengthMeters,
    double maxWavelengthMeters,
    double temperatureKelvin,
    std::size_t intervals)
{
    validatePositive(minWavelengthMeters, "min wavelength must be positive and finite");
    validatePositive(maxWavelengthMeters, "max wavelength must be positive and finite");
    validatePositive(temperatureKelvin, "temperature must be positive and finite");
    if (maxWavelengthMeters <= minWavelengthMeters) {
        throw std::invalid_argument("max wavelength must be greater than min wavelength");
    }
    if (intervals < 4) {
        intervals = 4;
    }
    if (intervals % 2 != 0) {
        intervals += 1;
    }

    const double h = (maxWavelengthMeters - minWavelengthMeters) / static_cast<double>(intervals);
    double sum = planck(minWavelengthMeters, temperatureKelvin) +
                 planck(maxWavelengthMeters, temperatureKelvin);

    for (std::size_t i = 1; i < intervals; ++i) {
        const double lambda = minWavelengthMeters + static_cast<double>(i) * h;
        const double weight = (i % 2 == 0) ? 2.0 : 4.0;
        sum += weight * planck(lambda, temperatureKelvin);
    }

    return (h / 3.0) * sum;
}
