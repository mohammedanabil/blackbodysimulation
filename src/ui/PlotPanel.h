#pragma once

#include "core/Spectrum.h"

struct AppState;

class PlotPanel {
public:
    static void render(
        const Spectrum& spectrum,
        const Spectrum* rayleighJeans,
        const AppState& state);
};
