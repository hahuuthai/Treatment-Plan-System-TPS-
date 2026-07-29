cmake_minimum_required(VERSION 3.16)
project(CTViewer)
# =========================
# C++ Settings
# =========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# =========================
# Qt
# =========================
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Concurrent
)
# =========================
# VTK
# =========================
find_package(VTK REQUIRED)
# =========================
# DCMTK (vcpkg)
# =========================
find_package(DCMTK CONFIG REQUIRED)
# =========================
# Sources
# =========================
set(SOURCES
    src/main.cpp
    
    "src/CTVolume/CTLoader.cpp"
    "src/CTVolume/SliceViewer.cpp"
    "src/CTVolume/MainWindow.cpp"
    "src/CTVolume/SliceRenderer.cpp"
    "src/CTVolume/WindowLevelController.cpp"
    "src/CTVolume/StructureOperationPanel.cpp"
    "src/CTVolume/StructureMarginPanel.cpp"
    "src/CTVolume/ColorPalette.cpp"
)
set(HEADERS
    "src/CTVolume/CTVolume.h"
    "src/CTVolume/CTLoader.h"
    "src/CTVolume/SliceViewer.h"
    "src/CTVolume/MainWindow.h"
    "src/CTVolume/SliceRenderer.h"
    "src/CTVolume/WindowLevelController.h"
    "src/CTVolume/StructureOperationPanel.h"
    "src/CTVolume/StructureMarginPanel.h"
    "src/CTVolume/ColorPalette.h"
)
# =========================
# Executable
# =========================
add_executable(CTViewer
    ${SOURCES}
    ${HEADERS}
    ${UIS}
 "src/RTSTRUCT/StructureModel.h"
 "src/RTSTRUCT/StructureSetContainer.h"
 "src/RTSTRUCT/RTStructLoader.h" 
 "src/RTSTRUCT/RTStructLoader.cpp" 
 "src/RTSTRUCT/StructureModel.cpp" 
 "src/RTSTRUCT/StructureSetContainer.cpp" 
 "src/RTSTRUCT/CoordinateTransformer.h" 
 "src/RTSTRUCT/CoordinateTransformer.cpp" 
 "src/RTSTRUCT/StructureListWidget.h" 
 "src/RTSTRUCT/StructureListWidget.cpp" 
 "src/RTSTRUCT/ContourVoxelizer.h" 
 "src/RTSTRUCT/ContourVoxelizer.cpp" 
 "src/RTSTRUCT/StructureVolumeCalculator.h" 
 "src/RTSTRUCT/StructureVolumeCalculator.cpp" 
 "src/RTSTRUCT/StructureBoolean.h" 
 "src/RTSTRUCT/StructureBoolean.cpp" 
 "src/RTSTRUCT/StructureMarginExpansion.h" 
 "src/RTSTRUCT/StructureMarginExpansion.cpp" "src/RTDOSE/DoseGridModel.h" "src/RTDOSE/DoseGridModel.cpp" "src/RTDOSE/DoseContainer.h" "src/RTDOSE/DoseContainer.cpp" "src/RTDOSE/RTDoseLoader.cpp" "src/RTDOSE/CoordinateAlignment.h" "src/RTDOSE/CoordinateAlignment.cpp" "src/RTDOSE/DoseSampler.h" "src/RTDOSE/DoseSampler.cpp")
# =========================
# Include
# =========================
target_include_directories(CTViewer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
# =========================
# Link Libraries
# =========================
target_link_libraries(CTViewer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Concurrent
    ${VTK_LIBRARIES}
    DCMTK::dcmdata
    DCMTK::dcmimgle
    DCMTK::dcmrt
    DCMTK::ofstd
    DCMTK::oflog
)
# =========================
# VTK Init
# =========================
vtk_module_autoinit(
    TARGETS CTViewer
    MODULES ${VTK_LIBRARIES}
)
# =========================
# Windows fix
# =========================
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()