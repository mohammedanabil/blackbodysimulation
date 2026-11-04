#include "StarCatalog.h"

#include <stdexcept>

namespace {

const std::vector<Preset> presets{
    {"Sun", 5778.0},
    {"Incandescent Bulb",2800.0},
    {"Rigel",12100.0},
    {"Human Body",310.0},
    {"Earth",288.0}
};

} // namespace

const std::vector<Preset>& StarCatalog::stars()
{
    return ::presets;
}

const Preset& StarCatalog::star(std::size_t index)
{
    if (index >= ::presets.size()) {
        throw std::out_of_range("Preset index is out of range");
    }
    return ::presets[index];
}
