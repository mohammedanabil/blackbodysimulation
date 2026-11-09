#pragma once

#include <filesystem>
#include <vector>

class ExcelWavelengthImporter {
public:
    static std::vector<double> loadNanometers(
        const std::filesystem::path& filePath);
};
