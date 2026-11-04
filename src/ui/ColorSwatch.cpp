#include "ColorSwatch.h"

#include "core/ColorConverter.h"

#include <imgui.h>

void ColorSwatch::render(const RGB& color)
{
    ImGui::Begin("Blackbody Color");
    ImVec2 size(180.0f, 180.0f);
    const ImVec4 swatchColor(
        static_cast<float>(color.red),
        static_cast<float>(color.green),
        static_cast<float>(color.blue),
        1.0f);
    ImGui::ColorButton("Blackbody swatch", swatchColor, 0, size);
    ImGui::Text(
        "RGB: %.3f, %.3f, %.3f",
        color.red,
        color.green,
        color.blue);
    ImGui::End();
}
