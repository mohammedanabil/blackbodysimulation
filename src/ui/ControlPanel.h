#pragma once

struct AppState;

class ControlPanel {
public:
    struct Actions {
        bool exportCSV = false;
        bool exportExcel = false;
        bool exportPNG = false;
        bool estimateTemperature = false;
        bool emissivityChanged = false;
        bool browseExcelFile = false;
        bool loadExcelFile = false;
        bool generateExcelSpectrum = false;
        bool returnToSimulation = false;
    };

    static Actions render(AppState& state);
};
