#pragma once

#include <vector>
#include <memory>

#include "DoseGridModel.h"

class DoseContainer
{
public:

    DoseContainer();

    void clear();

    int size() const;

    bool empty() const;

    void addDose(
        const DoseGridModel& dose);

    void removeDose(
        int index);

    DoseGridModel&
        current();

    const DoseGridModel&
        current() const;

    DoseGridModel*
        get(
            int index);

    const DoseGridModel*
        get(
            int index) const;

    void setCurrentIndex(
        int index);

    int currentIndex() const;

private:

    std::vector<DoseGridModel> m_doses;

    int m_currentIndex = -1;
};