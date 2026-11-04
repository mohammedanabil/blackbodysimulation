#include "ColorConverter.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct CIETriplet {
    double wavelengthMeters = 0.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

std::vector<CIETriplet> loadCIE(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error(
            "Could not open CIE 1931 table: " + path.string());
    }

    std::vector<CIETriplet> rows;
    std::string line;
    std::getline(input, line);

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        const std::size_t firstComma = line.find(',');
        const std::size_t secondComma =
            line.find(',', firstComma == std::string::npos ? 0 : firstComma + 1);
        const std::size_t thirdComma =
            line.find(',', secondComma == std::string::npos ? 0 : secondComma + 1);
        if (firstComma == std::string::npos ||
            secondComma == std::string::npos ||
            thirdComma == std::string::npos) {
            throw std::runtime_error("Invalid CIE 1931 CSV row");
        }

        try {
            const double wavelengthNm = std::stod(line.substr(0, firstComma));
            const double x = std::stod(line.substr(firstComma + 1, secondComma - firstComma - 1));
            const double y = std::stod(line.substr(secondComma + 1, thirdComma - secondComma - 1));
            const double z = std::stod(line.substr(thirdComma + 1));
            rows.push_back({wavelengthNm * 1.0e-9, x, y, z});
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid numeric value in CIE 1931 CSV");
        }
    }

    if (rows.size() < 2) {
        throw std::runtime_error("CIE 1931 table contains too few rows");
    }
    return rows;
}

double interpolate(
    const std::vector<double>& wavelengths,
    const std::vector<double>& values,
    double wavelength)
{
    if (wavelength <= wavelengths.front()) {
        return values.front();
    }
    if (wavelength >= wavelengths.back()) {
        return values.back();
    }

    for (std::size_t index = 1; index < wavelengths.size(); ++index) {
        if (wavelength <= wavelengths[index]) {
            const double span = wavelengths[index] - wavelengths[index - 1];
            const double fraction =
                (wavelength - wavelengths[index - 1]) / span;
            return values[index - 1] +
                   fraction * (values[index] - values[index - 1]);
        }
    }
    return values.back();
}

double linearToSRGB(double value)
{
    value = std::clamp(value, 0.0, 1.0);
    if (value <= 0.0031308) {
        return 12.92 * value;
    }
    return 1.055 * std::pow(value, 1.0 / 2.4) - 0.055;
}

} // namespace

RGB ColorConverter::spectrumToRGB(
    const Spectrum& spectrum,
    const std::filesystem::path& cieTablePath)
{
    if (spectrum.wavelengths.size() != spectrum.intensities.size()) {
        throw std::invalid_argument(
            "Spectrum wavelength and intensity vectors must have equal sizes");
    }
    if (spectrum.wavelengths.empty()) {
        throw std::invalid_argument("Spectrum must contain at least one sample");
    }

    const std::vector<CIETriplet> cie = loadCIE(cieTablePath);

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    for (std::size_t index = 1; index < cie.size(); ++index) {
        const double wavelengthStart = cie[index - 1].wavelengthMeters;
        const double wavelengthEnd = cie[index].wavelengthMeters;
        const double bandWidth = wavelengthEnd - wavelengthStart;

        const double intensityStart =
            interpolate(spectrum.wavelengths, spectrum.intensities, wavelengthStart);
        const double intensityEnd =
            interpolate(spectrum.wavelengths, spectrum.intensities, wavelengthEnd);
        const double intensity = 0.5 * (intensityStart + intensityEnd);

        x += cie[index].x * intensity * bandWidth;
        y += cie[index].y * intensity * bandWidth;
        z += cie[index].z * intensity * bandWidth;
    }

    const double xyzSum = x + y + z;
    if (!std::isfinite(xyzSum) || xyzSum <= 0.0) {
        return {0.0, 0.0, 0.0};
    }

    const double linearRed =
        3.2404542 * x - 1.5371385 * y - 0.4985314 * z;
    const double linearGreen =
        -0.9692660 * x + 1.8760108 * y + 0.0415560 * z;
    const double linearBlue =
        0.0556434 * x - 0.2040259 * y + 1.0572252 * z;

    const double maximum = std::max({
        std::abs(linearRed),
        std::abs(linearGreen),
        std::abs(linearBlue)
    });
    if (maximum <= 0.0) {
        return {0.0, 0.0, 0.0};
    }

    return {
        linearToSRGB(linearRed / maximum),
        linearToSRGB(linearGreen / maximum),
        linearToSRGB(linearBlue / maximum)
    };
}
