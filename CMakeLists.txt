cmake_minimum_required(VERSION 3.16)

project(BlackbodySimulation LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

add_library(blackbody_core STATIC
    src/core/EmissivityModel.cpp
    src/core/PlanckModel.cpp
    src/core/Spectrum.cpp
    src/core/ColorConverter.cpp
    src/core/TemperatureEstimator.cpp
)
target_include_directories(blackbody_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

add_library(blackbody_data STATIC
    src/data/Exporter.cpp
    src/data/ExcelWavelengthImporter.cpp
    src/data/StarCatalog.cpp
    src/data/stb_image_write.cpp
)
target_link_libraries(blackbody_data PUBLIC blackbody_core)
target_link_libraries(blackbody_data PUBLIC xlsxwriter)
target_include_directories(blackbody_data
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

include(FetchContent)
FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.8.0.zip
)
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
)
FetchContent_Declare(
    glad
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v0.1.36.zip
)
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip
)
FetchContent_Declare(
    implot
    URL https://github.com/epezent/implot/archive/refs/tags/v0.16.zip
)
FetchContent_Declare(
    stb
    URL https://github.com/nothings/stb/archive/refs/heads/master.zip
)
FetchContent_Declare(
    xlsxwriter
    URL https://github.com/jmcnamara/libxlsxwriter/archive/refs/tags/v1.2.2.zip
)
FetchContent_Declare(
    xlsxio
    URL https://github.com/brechtsanders/xlsxio/archive/refs/tags/0.2.37.zip
)
FetchContent_Declare(
    expat
    URL https://github.com/libexpat/libexpat/archive/refs/tags/R_2_7_1.zip
)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Catch2 glfw glad imgui implot stb xlsxwriter)
FetchContent_GetProperties(xlsxio)
if(NOT xlsxio_POPULATED)
    FetchContent_Populate(xlsxio)
endif()

find_package(EXPAT QUIET)
if(EXPAT_FOUND)
    set(EXPAT_TARGET EXPAT::EXPAT)
else()
    FetchContent_GetProperties(expat)
    if(NOT expat_POPULATED)
        FetchContent_Populate(expat)
    endif()
    set(EXPAT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(EXPAT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(EXPAT_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(
        ${expat_SOURCE_DIR}/expat
        ${expat_BINARY_DIR}
    )
    set(EXPAT_TARGET expat::expat)
endif()
find_package(ZLIB REQUIRED)
add_library(xlsxio_minizip STATIC
    ${xlsxwriter_SOURCE_DIR}/third_party/minizip/ioapi.c
    ${xlsxwriter_SOURCE_DIR}/third_party/minizip/unzip.c
)
target_include_directories(xlsxio_minizip PUBLIC
    ${xlsxwriter_SOURCE_DIR}/third_party/minizip
)
target_link_libraries(xlsxio_minizip PUBLIC ZLIB::ZLIB)

add_library(xlsxio_read STATIC
    ${xlsxio_SOURCE_DIR}/lib/xlsxio_read.c
    ${xlsxio_SOURCE_DIR}/lib/xlsxio_read_sharedstrings.c
)
target_compile_definitions(xlsxio_read PUBLIC
    BUILD_XLSXIO
    BUILD_XLSXIO_STATIC
)
target_compile_definitions(xlsxio_read PRIVATE
    USE_MINIZIP
)
target_include_directories(xlsxio_read
    PUBLIC ${xlsxio_SOURCE_DIR}/include
    PRIVATE
        ${xlsxio_SOURCE_DIR}/lib
        ${xlsxwriter_SOURCE_DIR}/third_party
)
target_link_libraries(xlsxio_read PUBLIC xlsxio_minizip ${EXPAT_TARGET})
target_link_libraries(blackbody_data PUBLIC xlsxio_read)

target_include_directories(blackbody_data PUBLIC ${stb_SOURCE_DIR})

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui PUBLIC glfw)

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

add_executable(blackbody_app
    src/main.cpp
    src/ui/App.cpp
    src/ui/ControlPanel.cpp
    src/ui/PlotPanel.cpp
    src/ui/ColorSwatch.cpp
)
set_target_properties(blackbody_app PROPERTIES
    OUTPUT_NAME "Blackbody Simulation V2.7"
)
target_link_libraries(blackbody_app PRIVATE
    blackbody_core
    blackbody_data
    imgui
    implot
    glfw
    glad
)
if(WIN32)
    target_link_libraries(blackbody_app PRIVATE opengl32 comdlg32)
endif()
target_compile_definitions(blackbody_app PRIVATE
    BLACKBODY_CIE_TABLE="data/cie_1931.csv"
    BLACKBODY_DATA_DIR="data"
)
add_custom_command(TARGET blackbody_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:blackbody_app>/data
)
if(MINGW)
    if(EXPAT_LIBRARY MATCHES "\\.dll\\.a$")
        get_filename_component(EXPAT_LIBRARY_DIR "${EXPAT_LIBRARY}" DIRECTORY)
        get_filename_component(EXPAT_PREFIX "${EXPAT_LIBRARY_DIR}" DIRECTORY)
        find_file(EXPAT_RUNTIME_DLL
            NAMES libexpat-1.dll expat.dll
            PATHS "${EXPAT_PREFIX}/bin"
            NO_DEFAULT_PATH
        )
        if(NOT EXPAT_RUNTIME_DLL)
            message(FATAL_ERROR "Could not locate the Expat runtime DLL")
        endif()
        add_custom_command(TARGET blackbody_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${EXPAT_RUNTIME_DLL}"
                $<TARGET_FILE_DIR:blackbody_app>
        )
    endif()
    if(ZLIB_LIBRARY MATCHES "\\.dll\\.a$")
        get_filename_component(ZLIB_LIBRARY_DIR "${ZLIB_LIBRARY}" DIRECTORY)
        get_filename_component(ZLIB_PREFIX "${ZLIB_LIBRARY_DIR}" DIRECTORY)
        find_file(ZLIB_RUNTIME_DLL
            NAMES zlib1.dll libz.dll
            PATHS "${ZLIB_PREFIX}/bin"
            NO_DEFAULT_PATH
        )
        if(NOT ZLIB_RUNTIME_DLL)
            message(FATAL_ERROR "Could not locate the zlib runtime DLL")
        endif()
        add_custom_command(TARGET blackbody_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ZLIB_RUNTIME_DLL}"
                $<TARGET_FILE_DIR:blackbody_app>
        )
    endif()
endif()

add_executable(blackbody_tests
    tests/test_planck.cpp
    tests/test_export.cpp
    tests/test_color.cpp
    tests/test_catalog.cpp
    tests/test_emissivity.cpp
    tests/test_excel_import.cpp
)
target_compile_definitions(blackbody_tests PRIVATE
    BLACKBODY_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(blackbody_tests PRIVATE
    blackbody_core
    blackbody_data
    Catch2::Catch2WithMain
)

enable_testing()
add_test(NAME blackbody_tests COMMAND blackbody_tests)
