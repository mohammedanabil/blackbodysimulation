#pragma once

struct AppState;

class ControlPanel {
public:
    struct Actions {
        bool exportCSV = false;
        bool exportBatchCSV = false;
        bool exportExcel = false;
        bool exportPNG = false;
        bool estimateTemperature = false;
        bool emissivityChanged = false;
        bool browseExcelFile = false;
        bool loadExcelFile = false;
        bool generateExcelSpectrum = false;
        bool returnToSimulation = false;
        bool wavelengthRangeChanged = false;
        bool pinCurrentCurve = false;
        bool clearPinnedCurves = false;
    };

    static Actions render(AppState& state);
};
