#include "Exporter.h"

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <stb_image_write.h>
#include <xlsxwriter.h>

void Exporter::toCSV(
    const Spectrum& spectrum,
    const std::filesystem::path& outputPath)
{
    if (spectrum.wavelengths.size() != spectrum.exitances.size()) {
        throw std::invalid_argument(
            "Spectrum wavelength and exitance vectors must have equal sizes");
    }

    if (outputPath.empty()) {
        throw std::invalid_argument("CSV output path must not be empty");
    }

    const std::filesystem::path parentPath = outputPath.parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

    std::ofstream output(outputPath);
    if (!output.is_open()) {
        throw std::runtime_error(
            "Could not open CSV output file: " + outputPath.string());
    }

    output.exceptions(std::ios::failbit | std::ios::badbit);
    output << std::scientific << std::setprecision(17);
    output << "Wavelength (m),Wavelength (nm),Temperature (K),Spectral Exitance (W/m³)\n";
    for (std::size_t index = 0; index < spectrum.wavelengths.size(); ++index) {
        output << spectrum.wavelengths[index] << ','
               << spectrum.wavelengths[index] * 1.0e9 << ','
               << spectrum.temperatureKelvin << ','
               << spectrum.exitances[index] << '\n';
    }

    output.close();
    if (!output) {
        throw std::runtime_error(
            "Could not write CSV output file: " + outputPath.string());
    }
}

void Exporter::toExcel(
    const Spectrum& spectrum,
    const std::filesystem::path& outputPath)
{
    if (spectrum.wavelengths.size() != spectrum.exitances.size()) {
        throw std::invalid_argument(
            "Spectrum wavelength and exitance vectors must have equal sizes");
    }

    if (outputPath.empty()) {
        throw std::invalid_argument("Excel output path must not be empty");
    }

    const std::filesystem::path parentPath = outputPath.parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

    lxw_workbook* workbook = workbook_new(outputPath.string().c_str());
    if (workbook == nullptr) {
        throw std::runtime_error(
            "Could not open Excel output file: " + outputPath.string());
    }
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, "Spectrum");
    lxw_format* headerFormat = workbook_add_format(workbook);
    lxw_format* redFormat = workbook_add_format(workbook);
    format_set_bold(headerFormat);
    format_set_bg_color(redFormat, 0xFF0000);
    format_set_font_color(redFormat, 0xFFFFFF);

    worksheet_write_string(worksheet, 0, 0, "Wavelength (m)", headerFormat);
    worksheet_write_string(worksheet, 0, 1, "Wavelength (nm)", headerFormat);
    worksheet_write_string(worksheet, 0, 2, "Temperature (K)", headerFormat);
    worksheet_write_string(
        worksheet, 0, 3, "Spectral Exitance (W/m³)", headerFormat);

    std::size_t peakExitanceIndex = 0;
    for (std::size_t index = 0; index < spectrum.wavelengths.size(); ++index) {
        if (spectrum.exitances[index] > spectrum.exitances[peakExitanceIndex]) {
            peakExitanceIndex = index;
        }
    }

    for (std::size_t index = 0; index < spectrum.wavelengths.size(); ++index) {
        const lxw_row_t excelRow = static_cast<lxw_row_t>(index + 1);
        lxw_format* wavelengthFormat =
            index == peakExitanceIndex ? redFormat : nullptr;
        worksheet_write_number(
            worksheet, excelRow, 0, spectrum.wavelengths[index], wavelengthFormat);
        worksheet_write_number(
            worksheet, excelRow, 1,
            spectrum.wavelengths[index] * 1.0e9, wavelengthFormat);
        worksheet_write_number(
            worksheet, excelRow, 2, spectrum.temperatureKelvin, nullptr);
        worksheet_write_number(
            worksheet, excelRow, 3, spectrum.exitances[index], nullptr);
    }

    worksheet_set_column(worksheet, 0, 2, 18, nullptr);

    lxw_error result = workbook_close(workbook);
    if (result != LXW_NO_ERROR) {
        throw std::runtime_error(
            "Could not write Excel output file: " + outputPath.string());
    }
}

void Exporter::toPNG(
    const std::filesystem::path& outputPath,
    int width,
    int height,
    const std::vector<unsigned char>& rgbaPixels)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("PNG dimensions must be positive");
    }
    if (rgbaPixels.size() !=
        static_cast<std::size_t>(width * height) * 4) {
        throw std::invalid_argument(
            "RGBA pixel buffer size does not match PNG dimensions");
    }

    const std::filesystem::path parentPath = outputPath.parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

    std::vector<unsigned char> flipped = rgbaPixels;
    const std::size_t rowSize =
        static_cast<std::size_t>(width) * 4;
    for (int row = 0; row < height / 2; ++row) {
        unsigned char* top = flipped.data() + row * rowSize;
        unsigned char* bottom =
            flipped.data() + (height - 1 - row) * rowSize;
        for (std::size_t column = 0; column < rowSize; ++column) {
            std::swap(top[column], bottom[column]);
        }
    }

    if (stbi_write_png(
            outputPath.string().c_str(),
            width,
            height,
            4,
            flipped.data(),
            width * 4) == 0) {
        throw std::runtime_error(
            "Could not write PNG output file: " + outputPath.string());
    }
}
