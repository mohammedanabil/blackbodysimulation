#include "EmissivityModel.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

void validateQuery(double wavelengthNm, double temperatureKelvin)
{
    if (!std::isfinite(wavelengthNm) || wavelengthNm <= 0.0) {
        throw std::invalid_argument(
            "wavelength must be positive and finite");
    }
    if (!std::isfinite(temperatureKelvin) || temperatureKelvin <= 0.0) {
        throw std::invalid_argument(
            "temperature must be positive and finite");
    }
}

std::string trim(const std::string& value)
{
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

double parseNumber(const std::string& field)
{
    std::size_t parsedLength = 0;
    const double value = std::stod(trim(field), &parsedLength);
    if (parsedLength != trim(field).size() || !std::isfinite(value)) {
        throw std::invalid_argument("CSV value must be finite numeric data");
    }
    return value;
}

} // namespace

double IdealBlackbodyModel::getEmissivity(
    double lambda_nm,
    double temp_K) const
{
    validateQuery(lambda_nm, temp_K);
    return 1.0;
}

ConstantEmissivityModel::ConstantEmissivityModel(double emissivity)
{
    if (!std::isfinite(emissivity)) {
        throw std::invalid_argument("emissivity must be finite");
    }
    emissivity_ = std::clamp(emissivity, 0.0, 1.0);
}

double ConstantEmissivityModel::getEmissivity(
    double lambda_nm,
    double temp_K) const
{
    validateQuery(lambda_nm, temp_K);
    return emissivity_;
}

TabulatedEmissivityModel::TabulatedEmissivityModel(
    std::vector<EmissivityPoint> points)
    : points_(std::move(points))
{
}

TabulatedEmissivityModel TabulatedEmissivityModel::loadFromCSV(
    const std::filesystem::path& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open emissivity CSV file: " + filePath.string());
    }

    std::vector<EmissivityPoint> points;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        const std::string content = trim(line);
        if (content.empty() || content.front() == '#') {
            continue;
        }

        const std::size_t comma = content.find(',');
        if (comma == std::string::npos ||
            content.find(',', comma + 1) != std::string::npos) {
            throw std::runtime_error(
                "Invalid emissivity CSV row at line " +
                std::to_string(lineNumber) + " in " + filePath.string());
        }

        try {
            const double wavelengthNm =
                parseNumber(content.substr(0, comma));
            const double emissivity =
                parseNumber(content.substr(comma + 1));
            if (wavelengthNm <= 0.0) {
                throw std::invalid_argument(
                    "wavelength must be positive");
            }
            points.push_back({
                wavelengthNm,
                std::clamp(emissivity, 0.0, 1.0)
            });
        } catch (const std::invalid_argument& error) {
            throw std::runtime_error(
                "Invalid emissivity CSV data at line " +
                std::to_string(lineNumber) + " in " + filePath.string() +
                ": " + error.what());
        } catch (const std::out_of_range&) {
            throw std::runtime_error(
                "Emissivity CSV number out of range at line " +
                std::to_string(lineNumber) + " in " + filePath.string());
        }
    }
    if (file.bad()) {
        throw std::runtime_error(
            "Could not read emissivity CSV file: " + filePath.string());
    }
    if (points.size() < 2) {
        throw std::runtime_error(
            "Emissivity CSV must contain at least two data points: " +
            filePath.string());
    }

    std::sort(
        points.begin(),
        points.end(),
        [](const EmissivityPoint& left, const EmissivityPoint& right) {
            return left.wavelength_nm < right.wavelength_nm;
        });
    for (std::size_t index = 1; index < points.size(); ++index) {
        if (points[index - 1].wavelength_nm == points[index].wavelength_nm) {
            throw std::runtime_error(
                "Emissivity CSV contains duplicate wavelengths: " +
                filePath.string());
        }
    }

    return TabulatedEmissivityModel(std::move(points));
}

double TabulatedEmissivityModel::getEmissivity(
    double lambda_nm,
    double temp_K) const
{
    validateQuery(lambda_nm, temp_K);

    if (lambda_nm <= points_.front().wavelength_nm) {
        return points_.front().emissivity;
    }
    if (lambda_nm >= points_.back().wavelength_nm) {
        return points_.back().emissivity;
    }

    const auto upper = std::lower_bound(
        points_.begin(),
        points_.end(),
        lambda_nm,
        [](const EmissivityPoint& point, double wavelengthNm) {
            return point.wavelength_nm < wavelengthNm;
        });
    const EmissivityPoint& lower = *(upper - 1);
    const double fraction =
        (lambda_nm - lower.wavelength_nm) /
        (upper->wavelength_nm - lower.wavelength_nm);
    return lower.emissivity +
           fraction * (upper->emissivity - lower.emissivity);
}
