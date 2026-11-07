cmake_minimum_required(VERSION 3.16)

project(BlackbodySimulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

add_library(blackbody_core STATIC
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
    src/data/StarCatalog.cpp
    src/data/stb_image_write.cpp
)
target_link_libraries(blackbody_data PUBLIC blackbody_core)
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
FetchContent_MakeAvailable(Catch2 glfw glad imgui implot stb)

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
    OUTPUT_NAME "Blackbody Simulation V1.0"
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
    target_link_libraries(blackbody_app PRIVATE opengl32)
endif()
target_compile_definitions(blackbody_app PRIVATE
    BLACKBODY_CIE_TABLE="${CMAKE_CURRENT_SOURCE_DIR}/data/cie_1931.csv"
)
add_custom_command(TARGET blackbody_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:blackbody_app>/data
)

add_executable(blackbody_tests
    tests/test_planck.cpp
    tests/test_export.cpp
    tests/test_color.cpp
    tests/test_catalog.cpp
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
