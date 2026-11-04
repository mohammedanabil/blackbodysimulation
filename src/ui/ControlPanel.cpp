#include "ControlPanel.h"

#include "AppState.h"
#include "data/StarCatalog.h"

#include <imgui.h>

ControlPanel::Actions ControlPanel::render(AppState& state)
{
    Actions actions;
    ImGui::Begin("Controls");
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

    float temperature = static_cast<float>(state.temperature);
    ImGui::SliderFloat(
        "Temperature",
        &temperature,
        1000.0f,
        10000.0f,
        "%.0f K");
    state.temperature = static_cast<double>(temperature);
    ImGui::Checkbox("Show Rayleigh-Jeans approximation", &state.showRJ);
    ImGui::Checkbox("Logarithmic intensity scale", &state.logScale);
    actions.estimateTemperature = ImGui::Button("Estimate Temperature");
    if (state.hasTemperatureEstimate) {
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
    actions.exportCSV = ImGui::Button("Export Spectrum CSV");
    ImGui::SameLine();
    actions.exportPNG = ImGui::Button("Export Plot PNG");
    ImGui::End();
    return actions;
}
