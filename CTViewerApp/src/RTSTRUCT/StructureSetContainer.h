#pragma once

#include <vector>
#include <string>

#include "StructureModel.h"

// =============================================
// STRUCTURE SET CONTAINER
// =============================================

class StructureSetContainer
{
public:

    std::vector<StructureModel> structures;

public:

    // =========================================
    // ADD
    // =========================================

    void addStructure(
        const StructureModel& structure);

    // =========================================
    // REMOVE
    // =========================================

    void removeStructure(
        const std::string& name);

    // =========================================
    // FIND
    // =========================================

    StructureModel* getStructure(
        const std::string& name);

    // =========================================
    // GET BY INDEX
    // =========================================

    StructureModel* getStructureByIndex(
        int index);

    // =========================================
    // CLEAR
    // =========================================

    void clear();
};