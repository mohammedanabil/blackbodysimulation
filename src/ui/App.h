#pragma once

#include "AppState.h"

struct GLFWwindow;

class App {
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

private:
    GLFWwindow* window_ = nullptr;
};
