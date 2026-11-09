#include "ExcelWavelengthImporter.h"

#include <xlsxio_read.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace {

struct SheetData {
    std::vector<double> wavelengths;
    std::string error;
    bool firstValueSeen = false;
};

std::string trim(const char* value)
{
    const std::string text = value == nullptr ? std::string{} : value;
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

int processCell(
    std::size_t row,
    std::size_t column,
    const XLSXIOCHAR* value,
    void* callbackData)
{
    auto& data = *static_cast<SheetData*>(callbackData);
    if (column != 1) {
        if (!trim(value).empty()) {
            data.error = "Excel sheet must contain only one populated column";
            return 1;
        }
        return 0;
    }

    const std::string cell = trim(value);
    if (cell.empty()) {
        return 0;
    }

    char* end = nullptr;
    errno = 0;
    const double wavelength = std::strtod(cell.c_str(), &end);
    const bool validNumber =
        end != cell.c_str() &&
        *end == '\0' &&
        errno != ERANGE &&
        std::isfinite(wavelength);

    if (!data.firstValueSeen) {
        data.firstValueSeen = true;
        if (!validNumber && row == 1) {
            return 0;
        }
    }
    if (!validNumber || wavelength <= 0.0) {
        data.error =
            "Excel wavelength values must be positive finite numbers in nm";
        return 1;
    }

    data.wavelengths.push_back(wavelength);
    return 0;
}

int processRow(std::size_t, std::size_t maximumColumn, void* callbackData)
{
    auto& data = *static_cast<SheetData*>(callbackData);
    if (maximumColumn > 1) {
        data.error = "Excel sheet must contain only one populated column";
        return 1;
    }
    return 0;
}

} // namespace

std::vector<double> ExcelWavelengthImporter::loadNanometers(
    const std::filesystem::path& filePath)
{
    if (filePath.empty()) {
        throw std::invalid_argument("Excel file path must not be empty");
    }
    if (!std::filesystem::exists(filePath) ||
        !std::filesystem::is_regular_file(filePath)) {
        throw std::runtime_error(
            "Excel file does not exist: " + filePath.string());
    }
    std::string extension = filePath.extension().string();
    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    if (extension != ".xlsx") {
        throw std::invalid_argument("Input file must have an .xlsx extension");
    }

    const std::string filename = filePath.string();
    using Reader = std::unique_ptr<
        std::remove_pointer_t<xlsxioreader>,
        decltype(&xlsxioread_close)>;
    Reader reader(xlsxioread_open(filename.c_str()), &xlsxioread_close);
    if (!reader) {
        throw std::runtime_error(
            "Could not open Excel workbook: " + filename);
    }

    SheetData data;
    const int result = xlsxioread_process(
        reader.get(),
        nullptr,
        XLSXIOREAD_SKIP_ALL_EMPTY,
        processCell,
        processRow,
        &data);
    if (!data.error.empty()) {
        throw std::runtime_error(data.error);
    }
    if (result != 0) {
        throw std::runtime_error(
            "Could not read the first worksheet in: " + filename);
    }
    if (data.wavelengths.size() < 2) {
        throw std::runtime_error(
            "Excel sheet must contain at least two wavelength values");
    }

    std::sort(data.wavelengths.begin(), data.wavelengths.end());
    if (std::adjacent_find(
            data.wavelengths.begin(),
            data.wavelengths.end()) != data.wavelengths.end()) {
        throw std::runtime_error(
            "Excel sheet contains duplicate wavelength values");
    }
    return data.wavelengths;
}
