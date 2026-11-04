#include "Exporter.h"

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <stb_image_write.h>

void Exporter::toCSV(
    const Spectrum& spectrum,
    const std::filesystem::path& outputPath)
{
    if (spectrum.wavelengths.size() != spectrum.intensities.size()) {
        throw std::invalid_argument(
            "Spectrum wavelength and intensity vectors must have equal sizes");
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
    output << "wavelength_meters,intensity_si_per_spectrum\n";
    for (std::size_t index = 0; index < spectrum.wavelengths.size(); ++index) {
        output << spectrum.wavelengths[index] << ','
               << spectrum.intensities[index] << '\n';
    }

    output.close();
    if (!output) {
        throw std::runtime_error(
            "Could not write CSV output file: " + outputPath.string());
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
