# CTViewer – DICOM CT / RTSTRUCT / RTDOSE Viewer

A lightweight Treatment Planning System (TPS) prototype developed in **C++ / Qt** for learning radiotherapy image processing and dose calculation.

The project focuses on loading DICOM datasets, displaying CT images, importing RTSTRUCT/RTDOSE, visualizing dose distribution, and performing quantitative dose analysis.

---

# Features

## Milestone 1 – CT Volume & Viewer ✅

### DICOM CT Import
- Import a complete CT DICOM series.
- Automatically detect and sort slices.
- Build a 3D CT volume.
- Read patient and study information.

### CT Volume
- Store
  - voxel HU values
  - spacing
  - origin
  - direction matrix
  - Image Position Patient (IPP)

### Slice Viewer
- Display axial CT slices.
- Mouse wheel slice scrolling.
- Window / Level adjustment.
- Zoom & Pan.
- HU value display under mouse cursor.
- Patient information panel.

---

## Milestone 2 – RTSTRUCT Support ✅

### RTSTRUCT Import
- Parse RT Structure Set.
- Read all structures.

### Structure Processing
- Convert contour points into image coordinates.
- Rasterize contours into binary masks.
- Build 3D mask volume.

### Structure Viewer
- Display contour overlay.
- Toggle structure visibility.
- Different colors for each structure.

---

## Milestone 3 – RTDOSE Support ✅

### RTDOSE Import
- Load RT Dose DICOM.
- Build 3D Dose Grid.

Store

- Dose Grid Scaling
- Pixel Data
- Grid Frame Offset Vector
- Origin
- Direction
- Dose spacing

---

### Coordinate Alignment

Implemented full coordinate transformation between

CT Index
↓

Patient Coordinate (mm)
↓

Dose Grid Index

Supports

- different origins
- different spacing
- different slice thickness
- different image orientation

---

### Dose Sampler

Implemented two independent sampling pipelines.

#### Display Sampling

Optimized for interactive rendering.

Features

- Nearest Neighbor
- Trilinear interpolation
- Cache for repeated queries
- Fast enough for real-time mouse movement

Used for

- dose overlay
- cursor inspection
- slice rendering

---

#### Quantitative Sampling

Designed for clinical calculations.

Features

- Pure trilinear interpolation
- No cache
- Optional supersampling
- Higher precision

Used for

- Mean Dose
- Maximum Dose
- Minimum Dose
- Future DVH calculation

---

### Dose Statistics

Implemented

- Mean Dose
- Maximum Dose
- Minimum Dose

Calculated directly from

Mask Volume
↓

Coordinate Alignment
↓

Dose Sampler
↓

Dose Grid

---

# Current Architecture

```
DICOM CT
        │
        ▼
 CTVolume
        │
        ▼
Coordinate Alignment
        │
        ├──────────────► Display Sampling
        │                     │
        │                     ▼
        │               Dose Overlay
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

# Build

```bash
git clone https://github.com/yourname/CTViewer.git

mkdir build
cd build

cmake ..
cmake --build .
```

---

# Usage

## 1. Load CT

```
File
    → Open CT Folder
```

Select a DICOM CT folder.

---

## 2. Load RTSTRUCT

```
File
    → Import RTSTRUCT
```

The contours will be displayed on CT slices.

---

## 3. Load RTDOSE

```
File
    → Import RTDOSE
```

The dose grid will be registered to the CT coordinate system.

---

## 4. Inspect Dose

Move the mouse over the CT image.

The viewer displays

- CT HU
- Dose value
- Patient coordinate

---

## 5. Dose Statistics

Select a structure.

The program computes

- Mean Dose
- Maximum Dose
- Minimum Dose

using the Quantitative Sampling pipeline.

---

# Project Status

| Module | Status |
|---------|--------|
| CT Loader | ✅ |
| CT Viewer | ✅ |
| Window/Level | ✅ |
| RTSTRUCT Loader | ✅ |
| Contour Rasterization | ✅ |
| Mask Volume | ✅ |
| RTDOSE Loader | ✅ |
| Coordinate Alignment | ✅ |
| Dose Sampler | ✅ |
| Mean Dose | ✅ |
| Max Dose | ✅ |
| Min Dose | ✅ |
| Dose Overlay | 🚧 Improving |
| DVH | ⏳ Planned |

---

# Future Work

- Dose color wash overlay
- DVH (Dose Volume Histogram)
- Isodose line visualization
- Dose profile
- Multi-plan comparison
- RTPLAN support
- 3D volume rendering

---

# Author

Developed as part of a personal project for learning Treatment Planning System (TPS) algorithms, medical image processing, and radiotherapy dose analysis.
