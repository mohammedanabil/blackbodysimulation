#include "PlotPanel.h"

#include "AppState.h"

#include <imgui.h>
#include <implot.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

std::vector<double> wavelengthsInNanometers(const Spectrum& spectrum)
{
    std::vector<double> values;
    values.reserve(spectrum.wavelengths.size());
    for (double wavelength : spectrum.wavelengths) {
        values.push_back(wavelength * 1.0e9);
    }
    return values;
}

std::vector<double> plottedIntensities(
    const Spectrum& spectrum,
    bool logScale)
{
    std::vector<double> values;
    values.reserve(spectrum.intensities.size());
    for (double intensity : spectrum.intensities) {
        values.push_back(logScale
            ? std::log10(std::max(intensity, 1.0e-300))
            : intensity);
    }
    return values;
}

} // namespace

void PlotPanel::render(
    const Spectrum& spectrum,
    const Spectrum* rayleighJeans,
    const AppState& state)
{
    ImGui::Begin("Radiation Spectrum");

    if (ImPlot::BeginPlot(
            "Blackbody Spectrum",
            ImVec2(-1, -1),
            ImPlotFlags_NoMenus)) {
        ImPlot::SetupAxis(ImAxis_X1, "Wavelength (nm)");
        ImPlot::SetupAxis(
            ImAxis_Y1,
            state.logScale ? "Intensity (log10)" : "Spectral Radiance");
        ImPlot::SetupLegend(ImPlotLocation_NorthEast);

        const std::vector<double> wavelengths =
            wavelengthsInNanometers(spectrum);
        const std::vector<double> intensities =
            plottedIntensities(spectrum, state.logScale);
        ImPlot::PlotLine(
            "Planck",
            wavelengths.data(),
            intensities.data(),
            static_cast<int>(std::min(
                wavelengths.size(),
                intensities.size())));

        if (rayleighJeans != nullptr) {
            const std::vector<double> rjWavelengths =
                wavelengthsInNanometers(*rayleighJeans);
            const std::vector<double> rjIntensities =
                plottedIntensities(*rayleighJeans, state.logScale);
            ImPlot::PlotLine(
                "Rayleigh-Jeeans",
                rjWavelengths.data(),
                rjIntensities.data(),
                static_cast<int>(std::min(
                    rjWavelengths.size(),
                    rjIntensities.size())));
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}
