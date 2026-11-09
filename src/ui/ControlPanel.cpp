#include "ControlPanel.h"

#include "AppState.h"
#include "data/StarCatalog.h"

#include <imgui.h>
#include <algorithm>

ControlPanel::Actions ControlPanel::render(AppState& state)
{
    Actions actions;
    ImGui::Begin("Controls");
    ImGui::SeparatorText("Planck Curve from Excel");
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
        "Emissivity (0 < eps < 1)",
        &state.excelEmissivity,
        0.05,
        0.1,
        "%.4f");
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
        ImGui::BeginDisabled(!state.excelWavelengthsLoaded);
        actions.generateExcelSpectrum =
            ImGui::Button("Update Planck Curve");
        ImGui::EndDisabled();
        ImGui::SameLine();
        actions.returnToSimulation =
            ImGui::Button("Return to built-in spectrum");
    } else {
        ImGui::BeginDisabled(!state.excelWavelengthsLoaded);
        actions.generateExcelSpectrum =
            ImGui::Button("Generate Planck Curve");
        ImGui::EndDisabled();
    }

    ImGui::BeginDisabled(state.excelMode);
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

    const auto& stars = StarCatalog::stars();
    const char* selectedName =
        stars[state.selectedStar].name.data();
    if (ImGui::BeginCombo("Catalog", selectedName)) {
        for (std::size_t index = 0; index < stars.size(); ++index) {
            const bool selected = state.selectedStar ==
                                  static_cast<int>(index);
            if (ImGui::Selectable(stars[index].name.data(), selected)) {
                state.selectedStar = static_cast<int>(index);
                state.temperature = stars[index].temperatureKelvin;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    double manualTemperature = state.temperature;
    if (ImGui::InputDouble(
            "Temperature K",
            &manualTemperature,
            100.0,
            1000.0,
            "%.0f")) {
        manualTemperature = std::clamp(manualTemperature, 1000.0, 10000.0);
        state.temperature = manualTemperature;
    }
    float temperature = static_cast<float>(state.temperature);
    ImGui::SliderFloat(
        "Temperature",
        &temperature,
        1000.0f,
        10000.0f,
        "%.0f K");
    state.temperature = static_cast<double>(temperature);
    ImGui::Checkbox("Show Rayleigh-Jeans approximation", &state.showRJ);
    ImGui::Checkbox("Logarithmic spectral exitance scale", &state.logScale);
    ImGui::BeginDisabled(state.selectedMaterial >= 2);
    actions.estimateTemperature = ImGui::Button("Estimate Temperature");
    ImGui::EndDisabled();
    if (state.selectedMaterial >= 2) {
        ImGui::TextDisabled(
            "Temperature fitting supports ideal and gray-body spectra only.");
    }
    if (state.hasTemperatureEstimate && state.selectedMaterial < 2) {
        if (state.estimateSucceeded) {
            ImGui::Text(
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
    ImGui::EndDisabled();

    actions.exportExcel = ImGui::Button("Export Spectrum Excel");
    ImGui::SameLine();
    actions.exportCSV = ImGui::Button("Export CSV");
    ImGui::SameLine();
    actions.exportPNG = ImGui::Button("Export Plot PNG");
    ImGui::End();
    return actions;
}
