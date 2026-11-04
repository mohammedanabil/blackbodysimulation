#include "core/Spectrum.h"
#include "data/Exporter.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

TEST_CASE("Exporter writes spectrum data to a CSV file", "[export]")
{
    Spectrum spectrum{
        5778.0,
        100.0e-9,
        300.0e-9,
        {100.0e-9, 200.0e-9, 300.0e-9},
        {1.0, 2.5, 4.0}
    };

    const std::filesystem::path outputPath =
        std::filesystem::temp_directory_path() /
        "blackbody_phase2_export_test.csv";

    std::filesystem::remove(outputPath);
    Exporter::toCSV(spectrum, outputPath);

    REQUIRE(std::filesystem::exists(outputPath));

    std::ifstream input(outputPath);
    REQUIRE(input.is_open());

    std::string header;
    std::getline(input, header);
    REQUIRE(header == "wavelength_meters,intensity_si_per_spectrum");

    std::string firstRow;
    std::getline(input, firstRow);
    REQUIRE_FALSE(firstRow.empty());

    input.close();
    std::filesystem::remove(outputPath);
}

TEST_CASE("Exporter writes a PNG file", "[export]")
{
    const std::filesystem::path outputPath =
        std::filesystem::temp_directory_path() /
        "blackbody_phase5_export_test.png";
    const std::vector<unsigned char> pixels(
        2 * 2 * 4,
        255);

    std::filesystem::remove(outputPath);
    Exporter::toPNG(outputPath, 2, 2, pixels);

    std::ifstream input(outputPath, std::ios::binary);
    REQUIRE(input.is_open());

    unsigned char signature[8] = {};
    input.read(reinterpret_cast<char*>(signature), sizeof(signature));
    REQUIRE(input.gcount() == 8);
    REQUIRE(signature[0] == 0x89);
    REQUIRE(signature[1] == 0x50);
    REQUIRE(signature[2] == 0x4E);
    REQUIRE(signature[3] == 0x47);

    input.close();
    std::filesystem::remove(outputPath);
}
