#pragma once

#include "Spectrum.h"

#include <filesystem>

struct RGB {
    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;
};

struct XYZ {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

class ColorConverter {
public:
    static RGB spectrumToRGB(
        const Spectrum& spectrum,
        const std::filesystem::path& cieTablePath);
};
