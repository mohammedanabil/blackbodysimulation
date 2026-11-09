# Blackbody Radiation Simulation V2.5

A C++20 desktop simulation that visualizes blackbody and real-body spectral exitance, displays the
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
3. For each wavelength, it calculates Planck spectral exitance multiplied by
   the selected emissivity, `M_real(lambda,T) = epsilon(lambda,T) * M_planck(lambda,T)`.
4. It optionally calculates the Rayleigh-Jeans approximation for comparison.
5. Wien's displacement law is used to find the expected wavelength of maximum
   emission.
6. The spectrum is integrated with the CIE 1931 color matching functions and
   converted from XYZ to displayable sRGB values.
7. The temperature can be estimated from a measured spectrum using:
   - peak wavelength analysis with Wien's law, or
   - nonlinear least-squares fitting with the golden-section search.
8. The program exports the spectrum as CSV and the rendered plot as PNG.

### Spectral emissivity

Spectrum generation uses an `EmissivityModel` strategy. The default is an ideal
blackbody (`epsilon = 1`); the controls also provide a constant gray-body
emissivity and interpolated tabulated spectra. Tabulated CSV files contain
`wavelength_nm,emissivity` rows, with `#` comments and blank lines ignored.
Wavelengths are sorted at load time, emissivity values are clamped to `[0, 1]`,
and values outside the table range use the nearest endpoint. Invalid or
duplicate data is reported rather than silently ignored.

The included steel and aluminum spectra are illustrative examples, not
experimental reference data. Replace them with measured values before using
the application for quantitative material predictions. Temperature fitting is
disabled for these wavelength-dependent examples because the current fitter
assumes a wavelength-independent emissivity.

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
- Ideal, gray-body, and tabulated wavelength-dependent emissivity models
- Excel wavelength import for custom Planck curves with user-specified temperature and emissivity
- Spectrum export to CSV
- Window-frame export to PNG

## Architecture

The project has three layers:

- `Core`: physics models, spectrum representation, and color conversion
- `Data`: star presets and CSV/PNG export
- `UI`: GLFW window, ImGui controls, ImPlot rendering, and color swatch

Core has no UI or Data dependencies. UI depends on Core and Data, while Data
depends on Core and its spreadsheet/export libraries.

## Requirements

- C++20 compiler
- CMake 3.16 or newer
- Internet access for the first CMake configure
- Zlib development libraries; CMake downloads Expat automatically if it is not installed
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

### Excel wavelength import

Choose a `.xlsx` workbook containing a single populated column of wavelength
values in nanometers. A first-row header is optional. At least two distinct,
positive wavelength values are required; the importer sorts them and rejects
invalid numbers or additional populated columns. Enter a positive temperature
and an emissivity strictly between 0 and 1, then generate the curve. The
imported curve replaces the built-in plot until you return to the built-in
spectrum. Exports use the currently displayed spectrum.

The Windows V2.5 release package includes the application, its runtime DLLs, and
the `data` directory. Extract the package and launch the executable from the
extracted folder so the bundled color and material tables are available.

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
| XLSX I/O 0.2.37 | Reads wavelength data from Excel workbooks |
| Expat and zlib | Parse and decompress `.xlsx` workbooks |
| stb_image_write | Writes exported plot frames as PNG files |
| Catch2 3.8.0 | Runs the core and data unit tests |

CMake downloads the project libraries with `FetchContent` during the first
configure step. Expat is downloaded if it is not installed; zlib development
files must be available on the build system. Catch2 is used only by the
independent test executable; the application itself does not depend on it.
