#include "StarCatalog.h"

#include <stdexcept>

namespace {

const std::vector<Preset> presets{
    {"Sun",                                5778.0, "Yellow dwarf star (G2V)"},
    {"Sirius A",                           9940.0, "Brightest star in night sky (A1V)"},
    {"Rigel",                             12100.0, "Blue supergiant in Orion (B8Ia)"},
    {"Betelgeuse",                         3500.0, "Red supergiant in Orion (M1-2Ia)"},
    {"Polaris",                            6015.0, "North Star (F7Ib)"},
    {"Incandescent Bulb",                  2800.0, "Tungsten filament thermal lamp"},
    {"Human Body",                          310.0, "Human skin thermal IR (~37°C)"},
    {"Earth",                               288.0, "Planetary surface mean temp (~15°C)"},
    {"CMB (Cosmic Microwave Background)",     2.725, "Big Bang relic radiation (peak ~1.06 mm)"}
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
