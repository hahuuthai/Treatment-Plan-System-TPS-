# CTViewer – DICOM CT / RTSTRUCT / RTDOSE Viewer

A lightweight **Treatment Planning System (TPS)** prototype developed in **C++17 / Qt** for learning radiotherapy treatment planning, medical image processing, and dose calculation.

The application supports loading DICOM CT images, RT Structure Sets (RTSTRUCT), RT Dose (RTDOSE), visualizing treatment data, and performing quantitative dose analysis through coordinate transformation and dose interpolation.

---

# Features

## CT DICOM Support

### DICOM Import

- Import an entire CT DICOM series
- Automatically detect and sort image slices
- Construct a 3D CT volume
- Read patient and study metadata

### CT Volume

Store complete CT volume information:

- Hounsfield Unit (HU) voxel values
- Image spacing
- Image origin
- Image orientation (Direction Cosines)
- Image Position Patient (IPP)

---

## CT Viewer

Interactive CT image viewer featuring

- Axial slice visualization
- Mouse wheel slice navigation
- Window / Level adjustment
- Zoom
- Pan
- HU value inspection
- Patient information display

---

## RT Structure Support

### RTSTRUCT Import

- Parse RT Structure Set DICOM files
- Load all available structures

### Structure Processing

- Convert contour points into CT image coordinates
- Rasterize contours into binary masks
- Generate 3D mask volumes

### Visualization

- Display contour overlays
- Toggle structure visibility
- Individual color assignment for each structure

---

## RT Dose Support

### RTDOSE Import

- Load RT Dose DICOM
- Construct a 3D dose grid

Store

- Dose Grid Scaling
- Dose voxel values
- Grid Frame Offset Vector
- Origin
- Direction Cosines
- Dose spacing

---

## Coordinate Transformation

Implemented a complete coordinate transformation pipeline between

```
CT Index
      ↓
Patient Coordinate (mm)
      ↓
Dose Grid Index
```

Supports

- Different image origins
- Different voxel spacing
- Different slice thickness
- Different image orientations

The coordinate transformation guarantees correct mapping between CT images and RT Dose regardless of acquisition geometry.

---

## Dose Sampling

Implemented two independent dose sampling pipelines.

### Display Sampling

Optimized for interactive visualization.

Features

- Nearest Neighbor interpolation
- Trilinear interpolation
- Cached sampling
- Real-time performance

Used for

- Dose overlay
- Mouse cursor inspection
- Interactive slice rendering

---

### Quantitative Sampling

Designed for accurate clinical dose calculation.

Features

- Pure trilinear interpolation
- No caching
- Optional supersampling
- High precision

Used for

- Mean Dose
- Maximum Dose
- Minimum Dose
- Future DVH calculation

---

## Dose Statistics

Implemented

- Mean Dose
- Maximum Dose
- Minimum Dose

Calculation workflow

```
Mask Volume
      ↓
Coordinate Transformation
      ↓
Dose Sampler
      ↓
Dose Grid
```

---

# System Architecture

```
                    DICOM CT
                        │
                        ▼
                   CT Volume
                        │
                        ▼
            Coordinate Transformation
                │                 │
                │                 ▼
                │         Display Sampling
                │                 │
                │                 ▼
                │       Dose Visualization
                │
                ▼
        Quantitative Sampling
                │
                ▼
      Mean / Max / Min Dose
```

---

# Technologies

- C++17
- Qt
- DCMTK
- CMake
- Visual Studio 2022

---

# Project Structure

```
Treatment-Plan-System-TPS
│
├── CTViewerApp/
├── src/
├── CMakeLists.txt
├── CMakePresets.json
└── README.md
```

---

# Build

## Requirements

- Visual Studio 2022
- CMake 3.16+
- Qt 6.x
- DCMTK
- C++17

---

## Clone

```bash
git clone https://github.com/hahuuthai/Treatment-Plan-System-TPS-.git
cd Treatment-Plan-System-TPS-
```

---

## Configure

```bash
mkdir build
cd build

cmake ..
```

---

## Build

```bash
cmake --build . --config Release
```

---

## Run

The executable will be generated inside the build output directory.

Example

```
build/Release/CTViewerApp.exe
```

---

# Usage

## Load CT Images

```
File
└── Open CT Folder
```

Select a folder containing a CT DICOM series.

---

## Import RTSTRUCT

```
File
└── Import RTSTRUCT
```

Structure contours will automatically be displayed on CT slices.

---

## Import RTDOSE

```
File
└── Import RTDOSE
```

The RT Dose grid is automatically registered to the CT coordinate system.

---

## Inspect Dose

Move the mouse over the CT image.

The viewer displays

- CT HU value
- Dose value
- Patient coordinates

---

## Calculate Dose Statistics

Select a structure.

The program calculates

- Mean Dose
- Maximum Dose
- Minimum Dose

using the Quantitative Sampling pipeline.

---

# Project Status

| Module | Status |
|-----------------------------|:------:|
| CT DICOM Import | ✅ |
| CT Volume Construction | ✅ |
| CT Viewer | ✅ |
| Window / Level | ✅ |
| RTSTRUCT Import | ✅ |
| Contour Rasterization | ✅ |
| 3D Mask Volume | ✅ |
| RTDOSE Import | ✅ |
| Coordinate Transformation | ✅ |
| Dose Sampling | ✅ |
| Mean Dose | ✅ |
| Maximum Dose | ✅ |
| Minimum Dose | ✅ |
| Dose Overlay | 🚧 Improving |
| DVH | ⏳ Planned |
| RTPLAN Support | ⏳ Planned |
| 3D Volume Rendering | ⏳ Planned |

---

# Future Work

- Dose Color Wash Overlay
- Dose Volume Histogram (DVH)
- Isodose Line Visualization
- Dose Profile Analysis
- RTPLAN Support
- Multi-plan Comparison
- 3D Volume Rendering
- GPU acceleration for dose visualization

---

# Author

Developed as a personal project for studying

- Treatment Planning Systems (TPS)
- DICOM RT standards
- Medical image processing
- Coordinate transformation
- Dose interpolation
- Radiotherapy dose analysis
