# Blackbody Radiation Simulation V3.0

A high-performance C++20 desktop scientific simulation for studying blackbody and real-body radiation,
investigating the resolution of the Ultraviolet Catastrophe, analyzing quantum vs. classical radiation models,
and exporting publication-grade spectra.

## Physics & Mathematical Foundations

### 1. Planck Spectral Exitance (Quantum Solution)

For wavelength $\lambda$ (meters) and absolute temperature $T$ (Kelvin):

$$M_\lambda(\lambda, T) = \frac{2\pi h c^2}{\lambda^5} \frac{1}{\exp\left(\frac{hc}{\lambda k_B T}\right) - 1}$$

- Planck's constant $h = 6.62607015 \times 10^{-34} \text{ J s}$
- Speed of light $c = 299792458 \text{ m/s}$
- Boltzmann constant $k_B = 1.380649 \times 10^{-23} \text{ J/K}$
- Stefan-Boltzmann constant $\sigma = 5.670374419 \times 10^{-8} \text{ W/(m}^2\text{ K}^4\text{)}$
- Wien displacement constant $b = 2.897771955 \times 10^{-3} \text{ m K}$

### 2. Classical Approximations & The Ultraviolet Catastrophe

- **Rayleigh-Jeans Law (Classical Equipartition)**:
  $$M_\lambda(\lambda, T) = \frac{2\pi c k_B T}{\lambda^4}$$
  Predicts infinite energy as $\lambda \to 0$ (the **Ultraviolet Catastrophe**), yet asymptotically agrees with Planck at long wavelengths ($\lambda \gg hc/k_B T$).

- **Wien's Radiation Approximation (1896)**:
  $$M_\lambda(\lambda, T) = \frac{2\pi h c^2}{\lambda^5} \exp\left(-\frac{hc}{\lambda k_B T}\right)$$
  Agrees with Planck in the short-wavelength (UV) limit but underestimates exitance at long wavelengths (IR / radio).

### 3. Integrated Quantities & Energy Partition

- **Stefan-Boltzmann Total Exitance**:
  $$M_{\text{total}} = \int_0^\infty M_\lambda d\lambda = \sigma T^4$$

- **Band Power & Energy Fractions**:
  The application computes band exitance using adaptive composite Simpson numerical integration:
  - **UV fraction**: $\int_0^{380\text{ nm}} M_\lambda d\lambda / M_{\text{total}}$
  - **Visible fraction**: $\int_{380\text{ nm}}^{780\text{ nm}} M_\lambda d\lambda / M_{\text{total}}$
  - **Infrared/Microwave fraction**: $\int_{780\text{ nm}}^\infty M_\lambda d\lambda / M_{\text{total}}$

---

## Features

- **Multi-Model Comparison**: Real-time simultaneous plotting of Quantum Planck, Classical Rayleigh-Jeans ($1/\lambda^4$), and Wien's approximation.
- **Ultraviolet Catastrophe Analysis Panel**: Dedicated interactive table showing exact values, divergence alerts, and percentage errors across Deep UV to Far IR.
- **Flexible Spectral Window**: Adjust wavelength limits from nanometers to millimeters, or use one-click buttons (`UV-Vis-NIR`, `Visible`, `Auto-Peak Window`).
- **Physical Quantities Panel**: Displays Wien peak ($\lambda_{\max}$), peak exitance, Stefan-Boltzmann total power, and UV/Visible/IR energy percentages.
- **Multi-Temperature Curve Overlay**: Pin and overlay multiple spectra at different temperatures on the same plot for direct visual comparison.
- **Dynamic Heating/Cooling Animation**: Smooth real-time animation with adjustable speed to observe spectral shifts and color changes.
- **Expanded Presets**: Includes the Sun (G2V), Sirius A, Rigel, Betelgeuse, Polaris, Human Body, Earth, and Cosmic Microwave Background (CMB, 2.725 K).
- **Physical Color Swatch**: Accurately computes true visible color via CIE 1931 color matching functions.
- **Comprehensive Data Export**:
  - Individual Spectrum to CSV and formatted XLSX
  - Batch temperature sweep export across multiple temperatures
  - High-resolution plot export to PNG

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

The Windows V2.7 release package includes the application, its runtime DLLs, and
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
