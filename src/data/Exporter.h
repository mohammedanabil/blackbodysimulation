#pragma once

#include "core/Spectrum.h"

#include <filesystem>
#include <vector>

class Exporter {
public:
    static void toCSV(const Spectrum& spectrum, const std::filesystem::path& outputPath);
    static void toExcel(const Spectrum& spectrum, const std::filesystem::path& outputPath);
    static void toPNG(
        const std::filesystem::path& outputPath,
        int width,
        int height,
        const std::vector<unsigned char>& rgbaPixels);
};
