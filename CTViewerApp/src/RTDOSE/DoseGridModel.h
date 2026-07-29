#pragma once

#include <vector>
#include <string>

class DoseGridModel
{
public:

    DoseGridModel();

    //-------------------------------------------------
    // Grid Size
    //-------------------------------------------------

    int width = 0;
    int height = 0;
    int depth = 0;

    //-------------------------------------------------
    // Geometry
    //-------------------------------------------------

    double spacing[3] =
    {
        1.0,
        1.0,
        1.0
    };

    double origin[3] =
    {
        0.0,
        0.0,
        0.0
    };

    // ImageOrientationPatient
    double direction[6] =
    {
        1.0,0.0,0.0,
        0.0,1.0,0.0
    };

    //-------------------------------------------------
    // Dose Values (Gy)
    //-------------------------------------------------

    std::vector<float> dose;

    double doseGridScaling = 1.0;

    //-------------------------------------------------
    // GridFrameOffsetVector
    //-------------------------------------------------

    std::vector<double> frameOffsets;

    //-------------------------------------------------
    // Dose Information
    //-------------------------------------------------

    std::string doseUnit = "GY";

    std::string doseType = "PHYSICAL";

    std::string doseSummationType = "PLAN";

    //-------------------------------------------------
    // DICOM UID
    //-------------------------------------------------

    std::string frameOfReferenceUID;

    std::string studyInstanceUID;

    std::string seriesInstanceUID;

    std::string sopInstanceUID;

    //-------------------------------------------------
    // Access
    //-------------------------------------------------

    bool isValid() const;

    bool contains(
        int x,
        int y,
        int z) const;

    void clear();

    void resize(
        int w,
        int h,
        int d);

    int index(
        int x,
        int y,
        int z) const;

    float& at(
        int x,
        int y,
        int z);

    const float& at(
        int x,
        int y,
        int z) const;

    float getMaximumDose() const;

    float getMinimumDose() const;

};