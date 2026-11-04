#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

struct Preset {
    std::string_view name;
    double temperatureKelvin = 0.0;
};

namespace StarCatalog {

const std::vector<Preset>& stars();
const Preset& star(std::size_t index);

}
