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

std::vector<double> plottedExitances(
    const Spectrum& spectrum,
    bool logScale)
{
    std::vector<double> values;
    values.reserve(spectrum.exitances.size());
    for (double exitance : spectrum.exitances) {
        values.push_back(logScale
            ? std::log10(std::max(exitance, 1.0e-300))
            : exitance);
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
            state.logScale ? "Spectral Exitance (W/m³, log10)" : "Spectral Exitance (W/m³)");
        ImPlot::SetupLegend(ImPlotLocation_NorthEast);

        const std::vector<double> wavelengths =
            wavelengthsInNanometers(spectrum);
        const std::vector<double> exitances =
            plottedExitances(spectrum, state.logScale);
        ImPlot::PlotLine(
            state.excelMode
                ? "Excel wavelengths: Planck x emissivity"
                : (state.selectedMaterial == 0 ? "Planck" : "Real body"),
            wavelengths.data(),
            exitances.data(),
            static_cast<int>(std::min(
                wavelengths.size(),
                exitances.size())));

        if (rayleighJeans != nullptr) {
            const std::vector<double> rjWavelengths =
                wavelengthsInNanometers(*rayleighJeans);
            const std::vector<double> rjExitances =
                plottedExitances(*rayleighJeans, state.logScale);
            ImPlot::PlotLine(
                "Rayleigh-Jeeans",
                rjWavelengths.data(),
                rjExitances.data(),
                static_cast<int>(std::min(
                    rjWavelengths.size(),
                    rjExitances.size())));
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}
