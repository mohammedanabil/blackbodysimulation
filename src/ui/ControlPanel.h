#pragma once

struct AppState;

class ControlPanel {
public:
    struct Actions {
        bool exportCSV = false;
        bool exportExcel = false;
        bool exportPNG = false;
        bool estimateTemperature = false;
    };

    static Actions render(AppState& state);
};
