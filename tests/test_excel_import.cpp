#include "data/ExcelWavelengthImporter.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <xlsxwriter.h>

#include <filesystem>
#include <string>
#include <vector>

namespace {

std::filesystem::path makeWorkbook(
    const std::string& name,
    bool includeHeader,
    bool includeSecondColumn = false,
    bool duplicateWavelength = false)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / name;
    std::filesystem::remove(path);

    lxw_workbook* workbook = workbook_new(path.string().c_str());
    REQUIRE(workbook != nullptr);
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, nullptr);
    REQUIRE(worksheet != nullptr);

    lxw_row_t row = 0;
    if (includeHeader) {
        worksheet_write_string(worksheet, row++, 0, "wavelength_nm", nullptr);
    }
    worksheet_write_number(worksheet, row++, 0, 700.0, nullptr);
    worksheet_write_number(worksheet, row++, 0, 500.0, nullptr);
    worksheet_write_number(
        worksheet,
        row,
        0,
        duplicateWavelength ? 500.0 : 600.0,
        nullptr);
    if (includeSecondColumn) {
        worksheet_write_string(worksheet, 0, 1, "unexpected", nullptr);
    }
    REQUIRE(workbook_close(workbook) == LXW_NO_ERROR);
    return path;
}

} // namespace

TEST_CASE("Excel wavelength importer reads and sorts a headed column", "[excel]")
{
    const std::filesystem::path path =
        makeWorkbook("blackbody_wavelength_import_test.xlsx", true);
    const std::vector<double> wavelengths =
        ExcelWavelengthImporter::loadNanometers(path);

    REQUIRE(wavelengths.size() == 3);
    REQUIRE(wavelengths[0] == Catch::Approx(500.0));
    REQUIRE(wavelengths[1] == Catch::Approx(600.0));
    REQUIRE(wavelengths[2] == Catch::Approx(700.0));
    std::filesystem::remove(path);
}

TEST_CASE("Excel wavelength importer accepts a headerless column", "[excel]")
{
    const std::filesystem::path path =
        makeWorkbook("blackbody_wavelength_headerless_test.xlsx", false);

    REQUIRE(ExcelWavelengthImporter::loadNanometers(path).size() == 3);
    std::filesystem::remove(path);
}

TEST_CASE("Excel wavelength importer rejects multiple populated columns", "[excel]")
{
    const std::filesystem::path path =
        makeWorkbook("blackbody_wavelength_extra_column_test.xlsx", true, true);

    REQUIRE_THROWS_AS(
        ExcelWavelengthImporter::loadNanometers(path),
        std::runtime_error);
    std::filesystem::remove(path);
}

TEST_CASE("Excel wavelength importer rejects duplicate wavelengths", "[excel]")
{
    const std::filesystem::path path =
        makeWorkbook(
            "blackbody_wavelength_duplicate_test.xlsx",
            true,
            false,
            true);

    REQUIRE_THROWS_AS(
        ExcelWavelengthImporter::loadNanometers(path),
        std::runtime_error);
    std::filesystem::remove(path);
}
