#include "ui/App.h"

#include <exception>
#include <iostream>

int main()
{
    try {
        App app;
        app.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Application error: " << error.what() << '\n';
        return 1;
    }
}
