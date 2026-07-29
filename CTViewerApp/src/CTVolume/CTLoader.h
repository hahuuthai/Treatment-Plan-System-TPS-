#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "CTVolume.h"

struct Slice
{
    std::string path;

    double position[3];

    double location;
};

struct Series
{
    std::string seriesUID;
    std::string description;

    std::string modality;
    std::string studyDate;
    std::string orientation;

    std::string patientID;
    std::string patientName;
    std::string studyDescription;
    std::string seriesNumber;
    std::string frameOfReferenceUID;

    double spacing[3] = { 1,1,1 };
    double sliceThickness = 0.0;

    double origin[3] = { 0, 0, 0 };
    double direction[6] = { 0 };

    double rescaleSlope = 1.0;
    double rescaleIntercept = 0.0;

    std::vector<Slice> slices;
};

struct Patient
{
    std::string name;
    std::map<std::string, Series> seriesMap;
};

using ProgressCallback = std::function<void(int, int, const std::string&)>;

class CTLoader
{
public:
    static bool scanFolder(
        const std::string& folder,
        std::map<std::string, Patient>& patients,
        ProgressCallback progress);

    static bool buildVolume(
        const Series& series,
        CTVolume& volume);
};