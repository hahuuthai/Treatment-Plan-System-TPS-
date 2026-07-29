#include "StructureSetContainer.h"

#include <algorithm>

// =============================================
// ADD
// =============================================

void StructureSetContainer::addStructure(
    const StructureModel& structure)
{
    structures.push_back(structure);
}

// =============================================
// REMOVE
// =============================================

void StructureSetContainer::removeStructure(
    const std::string& name)
{
    structures.erase(
        std::remove_if(
            structures.begin(),
            structures.end(),
            [&](const StructureModel& s)
            {
                return s.name == name;
            }),
        structures.end());
}

// =============================================
// FIND
// =============================================

StructureModel* StructureSetContainer::getStructure(
    const std::string& name)
{
    for (auto& s : structures)
    {
        if (s.name == name)
            return &s;
    }

    return nullptr;
}

// =============================================
// GET BY INDEX
// =============================================

StructureModel* StructureSetContainer::getStructureByIndex(
    int index)
{
    if (index < 0 ||
        index >= (int)structures.size())
    {
        return nullptr;
    }

    return &structures[index];
}

// =============================================
// CLEAR
// =============================================

void StructureSetContainer::clear()
{
    structures.clear();
}