#include "App.h"

#include "ControlPanel.h"
#include "PlotPanel.h"
#include "ColorSwatch.h"

#include "core/ColorConverter.h"
#include "data/Exporter.h"
#include "data/ExcelWavelengthImporter.h"
#include "core/EmissivityModel.h"
#include "core/PlanckModel.h"
#include "core/Spectrum.h"
#include "core/TemperatureEstimator.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <implot.h>

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#endif

namespace {

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

Spectrum generateRayleighJeans(
    const Spectrum& spectrum,
    const EmissivityModel& emissivityModel)
{
    Spectrum approximation = spectrum;
    approximation.exitances.clear();
    approximation.exitances.reserve(spectrum.wavelengths.size());
    for (double wavelength : spectrum.wavelengths) {
        approximation.exitances.push_back(
            emissivityModel.getEmissivity(
                wavelength * 1.0e9, spectrum.temperatureKelvin) *
            PlanckModel::rayleighJeans(
                wavelength, spectrum.temperatureKelvin));
    }
    return approximation;
}

#ifdef _WIN32
bool chooseExcelFile(std::array<char, 1024>& filePath)
{
    std::array<char, 32768> selectedPath{};
    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.lpstrFilter =
        "Excel Workbook (*.xlsx)\0*.xlsx\0All Files (*.*)\0*.*\0";
    dialog.lpstrFile = selectedPath.data();
    dialog.nMaxFile = static_cast<DWORD>(selectedPath.size());
    dialog.Flags =
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&dialog) == FALSE) {
        const DWORD error = CommDlgExtendedError();
        if (error != 0) {
            throw std::runtime_error(
                "Could not open the Excel file dialog (error " +
                std::to_string(error) + ")");
        }
        return false;
    }

    if (std::strlen(selectedPath.data()) >= filePath.size()) {
        throw std::runtime_error("Selected Excel file path is too long");
    }
    std::copy(
        selectedPath.begin(),
        selectedPath.begin() +
            static_cast<std::ptrdiff_t>(std::strlen(selectedPath.data()) + 1),
        filePath.begin());
    return true;
}
#endif

} // namespace

App::App()
{
    glfwSetErrorCallback([](int, const char* description) {
        if (description != nullptr) {
            std::cerr << "GLFW error: " << description << '\n';
        }
    });

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Could not initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(
        1280,
        720,
        "Blackbody Simulation V2.5",
        nullptr,
        nullptr);
    if (window_ == nullptr) {
        throw std::runtime_error("Could not create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

    if (!gladLoadGLLoader(
            reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        throw std::runtime_error("Could not initialize GLAD");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

App::~App()
{
    if (window_ == nullptr) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void App::run()
{
    AppState state;
    std::unique_ptr<EmissivityModel> emissivityModel =
        std::make_unique<IdealBlackbodyModel>();
    int activeMaterial = state.selectedMaterial;
    const auto makeEmissivityModel = [&](int selectedMaterial) {
        switch (selectedMaterial) {
        case 0:
            return std::unique_ptr<EmissivityModel>(
                std::make_unique<IdealBlackbodyModel>());
        case 1:
            return std::unique_ptr<EmissivityModel>(
                std::make_unique<ConstantEmissivityModel>(
                    state.constantEmissivity));
        case 2:
            return std::unique_ptr<EmissivityModel>(
                std::make_unique<TabulatedEmissivityModel>(
                    TabulatedEmissivityModel::loadFromCSV(
                        std::filesystem::path(BLACKBODY_DATA_DIR) /
                        "materials/oxidized_steel.csv")));
        case 3:
            return std::unique_ptr<EmissivityModel>(
                std::make_unique<TabulatedEmissivityModel>(
                    TabulatedEmissivityModel::loadFromCSV(
                        std::filesystem::path(BLACKBODY_DATA_DIR) /
                        "materials/polished_aluminum.csv")));
        default:
            throw std::invalid_argument("Unknown emissivity material");
        }
    };
    Spectrum spectrum = SpectrumGenerator::generate(
        state.temperature,
        100.0e-9,
        3000.0e-9,
        1200,
        *emissivityModel);
    double lastTemperature = state.temperature;
    Spectrum rayleighJeans;
    bool rayleighJeansValid = false;
    RGB currentColor =
        ColorConverter::spectrumToRGB(spectrum, BLACKBODY_CIE_TABLE);

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ControlPanel::Actions actions = ControlPanel::render(state);
        bool emissivityUpdated = false;
        bool refreshBuiltInSpectrum = false;
#ifdef _WIN32
        if (actions.browseExcelFile) {
            try {
                if (chooseExcelFile(state.excelFilePath)) {
                    state.excelError.clear();
                }
            } catch (const std::exception& error) {
                state.excelError = error.what();
                std::cerr << "Excel file dialog error: " << error.what()
                          << '\n';
            }
        }
#endif
        if (actions.loadExcelFile) {
            state.excelWavelengthsLoaded = false;
            state.excelWavelengthsNanometers.clear();
            try {
                state.excelWavelengthsNanometers =
                    ExcelWavelengthImporter::loadNanometers(
                        std::filesystem::path(state.excelFilePath.data()));
                state.excelWavelengthsLoaded = true;
                state.excelError.clear();
            } catch (const std::exception& error) {
                state.excelError = error.what();
                std::cerr << "Excel import error: " << error.what() << '\n';
            }
        }
        if (actions.generateExcelSpectrum) {
            try {
                if (!std::isfinite(state.excelTemperatureKelvin) ||
                    state.excelTemperatureKelvin <= 0.0) {
                    throw std::invalid_argument(
                        "Temperature must be positive and finite");
                }
                if (!std::isfinite(state.excelEmissivity) ||
                    state.excelEmissivity <= 0.0 ||
                    state.excelEmissivity >= 1.0) {
                    throw std::invalid_argument(
                        "Emissivity must be greater than 0 and less than 1");
                }
                const ConstantEmissivityModel excelEmissivity(
                    state.excelEmissivity);
                Spectrum importedSpectrum =
                    SpectrumGenerator::generateAtWavelengths(
                    state.excelTemperatureKelvin,
                    state.excelWavelengthsNanometers,
                    excelEmissivity);
                const RGB importedColor = ColorConverter::spectrumToRGB(
                    importedSpectrum, BLACKBODY_CIE_TABLE);
                spectrum = std::move(importedSpectrum);
                state.excelMode = true;
                state.activeExcelEmissivity = state.excelEmissivity;
                state.hasTemperatureEstimate = false;
                state.excelError.clear();
                currentColor = importedColor;
            } catch (const std::exception& error) {
                state.excelError = error.what();
                std::cerr << "Excel spectrum error: " << error.what() << '\n';
            }
        }
        if (actions.returnToSimulation) {
            state.excelMode = false;
            refreshBuiltInSpectrum = true;
        }
        if (actions.emissivityChanged) {
            try {
                emissivityModel = makeEmissivityModel(state.selectedMaterial);
                activeMaterial = state.selectedMaterial;
                state.emissivityError.clear();
                emissivityUpdated = true;
            } catch (const std::exception& error) {
                state.selectedMaterial = activeMaterial;
                state.emissivityError = error.what();
                std::cerr << "Emissivity model error: " << error.what() << '\n';
            }
        }
        if (!state.excelMode &&
            (refreshBuiltInSpectrum ||
             state.temperature != lastTemperature ||
             emissivityUpdated)) {
            spectrum = SpectrumGenerator::generate(
                state.temperature,
                100.0e-9,
                3000.0e-9,
                1200,
                *emissivityModel);
            lastTemperature = state.temperature;
            currentColor =
                ColorConverter::spectrumToRGB(spectrum, BLACKBODY_CIE_TABLE);
        }
        if (state.showRJ) {
            if (state.excelMode) {
                const ConstantEmissivityModel excelEmissivity(
                    state.activeExcelEmissivity);
                rayleighJeans =
                    generateRayleighJeans(spectrum, excelEmissivity);
            } else {
                rayleighJeans =
                    generateRayleighJeans(spectrum, *emissivityModel);
            }
            rayleighJeansValid = true;
        } else {
            rayleighJeansValid = false;
        }

        if (actions.estimateTemperature) {
            const TemperatureEstimate estimate =
                TemperatureEstimator::planckFit(spectrum);
            state.hasTemperatureEstimate = true;
            state.estimateSucceeded =
                estimate.status == TemperatureFitStatus::Ok;
            if (state.estimateSucceeded) {
                state.estimatedTemperature = estimate.temperatureKelvin;
                state.temperatureUncertainty = estimate.uncertaintyKelvin;
                state.estimatedEmissivity = estimate.emissivity;
                state.fitRmsResidual = estimate.rmsResidual;
            }
        }
        if (actions.exportCSV) {
            Exporter::toCSV(
                spectrum,
                std::filesystem::path("blackbody_spectrum.csv"));
        }
        if (actions.exportExcel) {
            Exporter::toExcel(
                spectrum,
                std::filesystem::path("blackbody_spectrum.xlsx"));
        }
        PlotPanel::render(
            spectrum,
            rayleighJeansValid ? &rayleighJeans : nullptr,
            state);
        ColorSwatch::render(currentColor);

        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window_, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (actions.exportPNG) {
            std::vector<unsigned char> pixels(
                static_cast<std::size_t>(displayWidth) *
                static_cast<std::size_t>(displayHeight) * 4);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(
                0,
                0,
                displayWidth,
                displayHeight,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                pixels.data());
            Exporter::toPNG(
                std::filesystem::path("blackbody_plot.png"),
                displayWidth,
                displayHeight,
                pixels);
        }

        glfwSwapBuffers(window_);
    }
}
