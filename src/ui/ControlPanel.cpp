#include "ControlPanel.h"

#include "AppState.h"
#include "data/StarCatalog.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>

#include "ControlPanel.h"

#include "AppState.h"
#include "core/Constants.h"
#include "data/StarCatalog.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>

ControlPanel::Actions ControlPanel::render(AppState& state)
{
    Actions actions;
    ImGui::Begin("Controls");

    // 1. Presets & Materials
    if (ImGui::CollapsingHeader("Presets & Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BeginDisabled(state.excelMode);
        const auto& stars = StarCatalog::stars();
        const char* selectedName = stars[state.selectedStar].name.data();
        if (ImGui::BeginCombo("Celestial / Presets", selectedName)) {
            for (std::size_t index = 0; index < stars.size(); ++index) {
                const bool selected = state.selectedStar == static_cast<int>(index);
                if (ImGui::Selectable(stars[index].name.data(), selected)) {
                    state.selectedStar = static_cast<int>(index);
                    state.temperature = stars[index].temperatureKelvin;

                    // If CMB preset is chosen, auto-scale wavelength window to millimeter range
                    if (stars[index].temperatureKelvin < 10.0) {
                        state.minWavelengthNm = 50000.0;    // 0.05 mm
                        state.maxWavelengthNm = 10000000.0; // 10.0 mm
                        actions.wavelengthRangeChanged = true;
                    }
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        const char* materials[] = {
            "Ideal Blackbody (eps=1.0)",
            "Gray Body (constant eps)",
            "Oxidized Steel (example CSV)",
            "Polished Aluminum (example CSV)"
        };
        if (ImGui::Combo(
                "Material Spectrum",
                &state.selectedMaterial,
                materials,
                IM_ARRAYSIZE(materials))) {
            actions.emissivityChanged = true;
        }
        if (state.selectedMaterial == 1) {
            constexpr double minimumEmissivity = 0.0;
            constexpr double maximumEmissivity = 1.0;
            if (ImGui::SliderScalar(
                    "Constant emissivity",
                    ImGuiDataType_Double,
                    &state.constantEmissivity,
                    &minimumEmissivity,
                    &maximumEmissivity,
                    "%.2f")) {
                actions.emissivityChanged = true;
            }
        }
        if (!state.emissivityError.empty()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "%s",
                state.emissivityError.c_str());
        }
        ImGui::EndDisabled();
    }

    // 2. Temperature & Animation
    if (ImGui::CollapsingHeader("Temperature & Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BeginDisabled(state.excelMode);
        double manualTemperature = state.temperature;
        if (ImGui::InputDouble(
                "Temperature (K)",
                &manualTemperature,
                50.0,
                500.0,
                "%.1f")) {
            manualTemperature = std::clamp(manualTemperature, 1.0, 50000.0);
            state.temperature = manualTemperature;
        }
        float temperature = static_cast<float>(state.temperature);
        if (ImGui::SliderFloat(
                "Quick Slider",
                &temperature,
                100.0f,
                30000.0f,
                "%.0f K")) {
            state.temperature = static_cast<double>(temperature);
        }

        // Animation control
        if (state.isAnimating) {
            if (ImGui::Button("Pause Animation ||")) {
                state.isAnimating = false;
            }
        } else {
            if (ImGui::Button("Animate Heating/Cooling >")) {
                state.isAnimating = true;
            }
        }
        ImGui::SameLine();
        float animSpeed = static_cast<float>(state.animationSpeed);
        if (ImGui::SliderFloat("Speed (K/s)", &animSpeed, 50.0f, 2500.0f, "%.0f K/s")) {
            state.animationSpeed = static_cast<double>(animSpeed);
        }
        ImGui::EndDisabled();
    }

    // 3. Wavelength Window
    if (ImGui::CollapsingHeader("Wavelength Domain (Spectral Window)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BeginDisabled(state.excelMode);
        bool rangeChanged = false;
        double minNm = state.minWavelengthNm;
        double maxNm = state.maxWavelengthNm;

        if (ImGui::InputDouble("Min λ (nm)", &minNm, 10.0, 100.0, "%.1f")) {
            minNm = std::max(0.1, minNm);
            if (minNm < maxNm) {
                state.minWavelengthNm = minNm;
                rangeChanged = true;
            }
        }
        if (ImGui::InputDouble("Max λ (nm)", &maxNm, 50.0, 500.0, "%.1f")) {
            if (maxNm > minNm) {
                state.maxWavelengthNm = maxNm;
                rangeChanged = true;
            }
        }

        ImGui::TextDisabled("Quick Ranges:");
        if (ImGui::SmallButton("UV-Vis-NIR (100-3000nm)")) {
            state.minWavelengthNm = 100.0;
            state.maxWavelengthNm = 3000.0;
            rangeChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Visible (380-780nm)")) {
            state.minWavelengthNm = 380.0;
            state.maxWavelengthNm = 780.0;
            rangeChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Auto-Peak Window")) {
            const double peakNm = (Constants::wien / state.temperature) * 1.0e9;
            state.minWavelengthNm = std::max(1.0, peakNm * 0.1);
            state.maxWavelengthNm = peakNm * 5.0;
            rangeChanged = true;
        }

        if (rangeChanged) {
            actions.wavelengthRangeChanged = true;
        }
        ImGui::EndDisabled();
    }

    // 4. Comparison Models & Catastrophe
    if (ImGui::CollapsingHeader("Radiation Models & Catastrophe", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Rayleigh-Jeans (Classical 1/λ⁴)", &state.showRJ);
        ImGui::Checkbox("Show Wien Approximation (Short-λ)", &state.showWien);
        ImGui::Checkbox("Ultraviolet Catastrophe Analysis Window", &state.showClassicalComparison);
        ImGui::Checkbox("Logarithmic spectral exitance scale", &state.logScale);
        ImGui::Checkbox("Highlight visible band (380-780 nm)", &state.showVisibleBand);
    }

    // 5. Derived Physical Quantities
    if (ImGui::CollapsingHeader("Physical Quantities (Derived)", ImGuiTreeNodeFlags_DefaultOpen)) {
        const double peakNm = state.physics.peakWavelengthMeters * 1.0e9;
        if (peakNm >= 1000000.0) {
            ImGui::Text("Wien Peak λ_max: %.3f mm", peakNm * 1.0e-6);
        } else if (peakNm >= 1000.0) {
            ImGui::Text("Wien Peak λ_max: %.3f um (%.1f nm)", peakNm * 1.0e-3, peakNm);
        } else {
            ImGui::Text("Wien Peak λ_max: %.2f nm", peakNm);
        }

        ImGui::Text("Peak Exitance: %.3e W/m³", state.physics.peakExitance);
        ImGui::Text("Total Power (Stefan-Boltzmann): %.3e W/m²", state.physics.totalPower);

        ImGui::Separator();
        ImGui::Text("Spectral Energy Distribution:");
        ImGui::TextColored(ImVec4(0.6f, 0.4f, 1.0f, 1.0f), "  UV (< 380 nm):     %.1f%%", state.physics.uvFraction * 100.0);
        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "  Visible (380-780): %.1f%%", state.physics.visibleFraction * 100.0);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "  Infrared (> 780):  %.1f%%", state.physics.irFraction * 100.0);
    }

    // 6. Multi-Temperature Comparison (Pinned Curves)
    if (ImGui::CollapsingHeader("Multi-Temperature Overlay", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Pin Current Curve")) {
            actions.pinCurrentCurve = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All Pinned") && !state.pinnedCurves.empty()) {
            actions.clearPinnedCurves = true;
        }

        if (state.pinnedCurves.empty()) {
            ImGui::TextDisabled("No pinned curves. Click 'Pin Current Curve' to overlay.");
        } else {
            for (std::size_t i = 0; i < state.pinnedCurves.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));
                ImGui::Checkbox(state.pinnedCurves[i].name.c_str(), &state.pinnedCurves[i].visible);
                ImGui::SameLine();
                if (ImGui::SmallButton("X")) {
                    state.pinnedCurves.erase(state.pinnedCurves.begin() + static_cast<std::ptrdiff_t>(i));
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
        }
    }

    // 7. Temperature Estimator
    if (ImGui::CollapsingHeader("Temperature Fitting & Estimation")) {
        ImGui::BeginDisabled(state.selectedMaterial >= 2 || state.excelMode);
        actions.estimateTemperature = ImGui::Button("Estimate Temperature");
        ImGui::EndDisabled();
        if (state.selectedMaterial >= 2) {
            ImGui::TextDisabled("Fitting supports ideal and gray-body spectra only.");
        }
        if (state.hasTemperatureEstimate && state.selectedMaterial < 2) {
            if (state.estimateSucceeded) {
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f),
                    "Estimated T: %.1f +/- %.1f K",
                    state.estimatedTemperature,
                    state.temperatureUncertainty);
                ImGui::Text(
                    "Emissivity: %.3f  RMS: %.4e",
                    state.estimatedEmissivity,
                    state.fitRmsResidual);
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                                   "Temperature estimate failed");
            }
        }
    }

    // 8. Excel Import
    if (ImGui::CollapsingHeader("Planck Curve from Excel (.xlsx)")) {
        ImGui::InputText(
            "Excel file (.xlsx)",
            state.excelFilePath.data(),
            state.excelFilePath.size());
#ifdef _WIN32
        ImGui::SameLine();
        actions.browseExcelFile = ImGui::Button("Browse...");
#endif
        ImGui::SameLine();
        actions.loadExcelFile = ImGui::Button("Load Wavelengths");
        ImGui::InputDouble(
            "Excel temperature (K)",
            &state.excelTemperatureKelvin,
            100.0,
            500.0,
            "%.2f");
        ImGui::InputDouble(
            "Emissivity (0 < eps <= 1)",
            &state.excelEmissivity,
            0.05,
            0.1,
            "%.4f");
        const bool validExcelEmissivity =
            std::isfinite(state.excelEmissivity) &&
            state.excelEmissivity > 0.0 &&
            state.excelEmissivity <= 1.0;
        if (!validExcelEmissivity) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "Error: Emissivity must be greater than 0 and at most 1.");
        }
        if (state.excelWavelengthsLoaded) {
            ImGui::Text(
                "Loaded %zu wavelength samples",
                state.excelWavelengthsNanometers.size());
        }
        if (!state.excelError.empty()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "%s",
                state.excelError.c_str());
        }
        if (state.excelMode) {
            ImGui::BeginDisabled(
                !state.excelWavelengthsLoaded || !validExcelEmissivity);
            actions.generateExcelSpectrum =
                ImGui::Button("Update Planck Curve");
            ImGui::EndDisabled();
            ImGui::SameLine();
            actions.returnToSimulation =
                ImGui::Button("Return to built-in spectrum");
        } else {
            ImGui::BeginDisabled(
                !state.excelWavelengthsLoaded || !validExcelEmissivity);
            actions.generateExcelSpectrum =
                ImGui::Button("Generate Planck Curve");
            ImGui::EndDisabled();
        }
    }

    // 9. Export
    ImGui::Separator();
    actions.exportExcel = ImGui::Button("Export Excel (.xlsx)");
    ImGui::SameLine();
    actions.exportCSV = ImGui::Button("Export CSV");
    ImGui::SameLine();
    actions.exportBatchCSV = ImGui::Button("Batch Export (1k-10k K)");
    ImGui::SameLine();
    actions.exportPNG = ImGui::Button("Export Plot PNG");
    ImGui::End();

    return actions;
}
