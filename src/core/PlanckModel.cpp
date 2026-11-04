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

    constexpr double logMaximumDouble = 709.782712893384;
    if (exponent > logMaximumDouble) {
        return 0.0;
    }

    const double numerator = 2.0 * Constants::planck *
                             Constants::speedOfLight * Constants::speedOfLight;
    const double wavelengthToFifth = std::pow(wavelengthMeters, 5);
    const double denominator = std::expm1(exponent);

    return numerator / (wavelengthToFifth * denominator);
}

double PlanckModel::rayleighJeans(double wavelengthMeters, double temperatureKelvin)
{
    validatePositive(wavelengthMeters, "wavelength must be positive and finite");
    validatePositive(temperatureKelvin, "temperature must be positive and finite");

    return 2.0 * Constants::speedOfLight * Constants::boltzmann * temperatureKelvin /
           std::pow(wavelengthMeters, 4);
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
