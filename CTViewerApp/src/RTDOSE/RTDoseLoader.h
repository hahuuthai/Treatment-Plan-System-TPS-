#pragma once

#include <string>
#include "DoseGridModel.h"

class RTDoseLoader
{
public:

    bool loadDose(
        const std::string& fileName,
        DoseGridModel& dose);
};