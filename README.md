# Blackbody Radiation Simulation

A C++20 desktop simulation that visualizes blackbody spectral exitance, displays the
derived visible color, compares the Rayleigh-Jeeans approximation, and exports
spectrum data and plot images.

## Physics

### Planck spectral exitance

For wavelength `lambda` in meters and absolute temperature `T` in kelvin:

```text
B_lambda(lambda, T) =
    2hc^2 / lambda^5
    --------------------------------
    exp(hc / lambda k_B T) - 1
```

The simulation uses SI units and `double` precision:

- `h = 6.62607015e-34 J s`
- `c = 299792458 m/s`
- `k_B = 1.380649e-23 J/K`
- `sigma = 5.670374419e-8 W/(m^2 K^4)`
- Wien displacement constant `b = 2.897771955e-3 m K`

Wien's law gives the spectral peak:

```text
lambda_max = b / T
```

Stefan-Boltzmann total power is:

```text
P/A = sigma * T^4
```

The Planck exponential is evaluated through overflow-safe logic. Values with
very large exponents are treated as effectively zero spectral exitance.

## Algorithm

1. The application accepts a temperature between 1000 K and 10000 K.
2. It generates 1200 wavelength samples uniformly from 100 nm to 3000 nm.
3. For each wavelength, it calculates spectral exitance with Planck's law.
4. It optionally calculates the Rayleigh-Jeans approximation for comparison.
5. Wien's displacement law is used to find the expected wavelength of maximum
   emission.
6. The spectrum is integrated with the CIE 1931 color matching functions and
   converted from XYZ to displayable sRGB values.
7. The temperature can be estimated from a measured spectrum using:
   - peak wavelength analysis with Wien's law, or
   - nonlinear least-squares fitting with the golden-section search.
8. The program exports the spectrum as CSV and the rendered plot as PNG.

The temperature-fitting algorithm searches logarithmic temperature values to
keep the search stable across a wide temperature range. It also estimates
emissivity when emissivity fitting is enabled and calculates the RMS residual
and temperature uncertainty.

## Color conversion

The sample spectrum is integrated against the CIE 1931 `x`, `y`, and `z` color
matching functions from `data/cie_1931.csv`. The resulting XYZ values are
converted to linear sRGB using the standard D65 transformation, peak-normalized,
gamma-corrected to sRGB, and clipped to valid channels.

This is a physically informed display approximation, not a colorimetric standard
rendering path. The sample range, interpolation method, normalization, and CIE
table resolution affect the resulting colors.

## Features

- Temperature control from 1000 K to 10000 K
- Sun, Betelgeuse, and Sirius presets
- Interactive ImPlot radiation curve
- Optional Rayleigh-Jeans approximation
- Optional logarithmic spectral exitance axis
- Derived blackbody color swatch
- Spectrum export to CSV
- Window-frame export to PNG

## Architecture

The project has three layers:

- `Core`: physics models, spectrum representation, and color conversion
- `Data`: star presets and CSV/PNG export
- `UI`: GLFW window, ImGui controls, ImPlot rendering, and color swatch

Core has no UI or Data dependencies. UI depends on Core and Data, while Data
depends on Core only.

## Requirements

- C++20 compiler
- CMake 3.16 or newer
- Internet access for the first CMake configure
- OpenGL 3.3-capable system
- Visual Studio or another supported Windows/Unix build environment

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run the application:

```bash
./build/Release/blackbody_app.exe
```

The executable name and configuration directory may differ on non-Windows
platforms or generators.

## Tests

```bash
ctest --test-dir build -C Debug --output-on-failure
```

Or run the test executable directly:

```bash
./build/Debug/blackbody_tests.exe
```

## Libraries

| Library | Purpose |
|---|---|
| GLFW 3.4 | Creates the desktop window, OpenGL context, and handles input |
| GLAD 0.1.36 | Loads OpenGL 3.3 functions |
| Dear ImGui 1.91.9b | Provides the controls, panels, buttons, and swatch interface |
| ImPlot 0.16 | Renders the radiation spectrum and comparison plots |
| stb_image_write | Writes exported plot frames as PNG files |
| Catch2 3.8.0 | Runs the core and data unit tests |

All external libraries are downloaded and configured automatically with CMake
`FetchContent` during the first configure step. Catch2 is used only by the
independent test executable; the application itself does not depend on it.
