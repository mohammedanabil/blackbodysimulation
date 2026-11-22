#include "PlotPanel.h"

#include "AppState.h"
#include "core/PlanckModel.h"

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
    const Spectrum* wienApproximation,
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

        if (state.showVisibleBand && state.minWavelengthNm <= 780.0 && state.maxWavelengthNm >= 380.0) {
            static const double visibleBounds[2] = {380.0, 780.0};
            ImPlot::PlotInfLines("Visible Band (380-780nm)", visibleBounds, 2);
        }

        const std::vector<double> wavelengths =
            wavelengthsInNanometers(spectrum);
        const std::vector<double> exitances =
            plottedExitances(spectrum, state.logScale);
        ImPlot::PlotLine(
            state.excelMode
                ? "Excel wavelengths: Planck x emissivity"
                : (state.selectedMaterial == 0 ? "Planck (Quantum)" : "Real body"),
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
                "Rayleigh-Jeans (Classical)",
                rjWavelengths.data(),
                rjExitances.data(),
                static_cast<int>(std::min(
                    rjWavelengths.size(),
                    rjExitances.size())));
        }

        if (wienApproximation != nullptr) {
            const std::vector<double> wienWavelengths =
                wavelengthsInNanometers(*wienApproximation);
            const std::vector<double> wienExitances =
                plottedExitances(*wienApproximation, state.logScale);
            ImPlot::PlotLine(
                "Wien Approx (Short-λ)",
                wienWavelengths.data(),
                wienExitances.data(),
                static_cast<int>(std::min(
                    wienWavelengths.size(),
                    wienExitances.size())));
        }

        for (const auto& pinned : state.pinnedCurves) {
            if (!pinned.visible || pinned.wavelengths.empty()) {
                continue;
            }
            std::vector<double> pinnedExitances;
            pinnedExitances.reserve(pinned.exitances.size());
            for (double ext : pinned.exitances) {
                pinnedExitances.push_back(state.logScale
                    ? std::log10(std::max(ext, 1.0e-300))
                    : ext);
            }
            ImPlot::PlotLine(
                pinned.name.c_str(),
                pinned.wavelengths.data(),
                pinnedExitances.data(),
                static_cast<int>(std::min(pinned.wavelengths.size(), pinnedExitances.size())));
        }

        ImPlot::EndPlot();
    }

    ImGui::End();

    if (state.showClassicalComparison) {
        ImGui::Begin("Classical vs Quantum: The Ultraviolet Catastrophe", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Historical & Mathematical Analysis at T = %.0f K", state.temperature);
        ImGui::Separator();
        ImGui::TextWrapped(
            "Classical mechanics (Equipartition Theorem) predicted Rayleigh-Jeans exitance proportional to 1/lambda^4, "
            "which diverges to infinity as lambda -> 0 (The Ultraviolet Catastrophe). "
            "Wien's approximation worked well in UV but failed at long wavelengths. "
            "Max Planck resolved this in 1900 by introducing energy quanta E = h*nu.");
        ImGui::Spacing();

        if (ImGui::BeginTable("CatastropheTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Wavelength");
            ImGui::TableSetupColumn("Spectral Region");
            ImGui::TableSetupColumn("Planck (Quantum)");
            ImGui::TableSetupColumn("Rayleigh-Jeans");
            ImGui::TableSetupColumn("Wien Approx");
            ImGui::TableHeadersRow();

            const struct SamplePoint {
                double lambdaNm;
                const char* region;
            } points[] = {
                { 100.0, "Deep UV" },
                { 250.0, "Near UV" },
                { 500.0, "Visible Green" },
                { 1000.0, "Near IR" },
                { 5000.0, "Mid IR" },
                { 20000.0, "Far IR" }
            };

            for (const auto& pt : points) {
                const double lambdaM = pt.lambdaNm * 1.0e-9;
                const double planckVal = PlanckModel::planck(lambdaM, state.temperature);
                const double rjVal = PlanckModel::rayleighJeans(lambdaM, state.temperature);
                const double wienVal = PlanckModel::wienApproximation(lambdaM, state.temperature);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%.0f nm", pt.lambdaNm);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(pt.region);
                ImGui::TableNextColumn();
                ImGui::Text("%.2e", planckVal);
                ImGui::TableNextColumn();
                if (rjVal > planckVal * 50.0 || !std::isfinite(rjVal)) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%.2e (DIVERGES!)", rjVal);
                } else {
                    const double errPct = ((rjVal - planckVal) / std::max(planckVal, 1.0e-300)) * 100.0;
                    ImGui::Text("%.2e (%+.0f%%)", rjVal, errPct);
                }
                ImGui::TableNextColumn();
                const double wienErrPct = ((wienVal - planckVal) / std::max(planckVal, 1.0e-300)) * 100.0;
                ImGui::Text("%.2e (%+.0f%%)", wienVal, wienErrPct);
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
}
