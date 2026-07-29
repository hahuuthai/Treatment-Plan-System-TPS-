#include "DoseContainer.h"

DoseContainer::DoseContainer()
{
}

void DoseContainer::clear()
{
    m_doses.clear();
    m_currentIndex = -1;
}

int DoseContainer::size() const
{
    return
        static_cast<int>(
            m_doses.size());
}

bool DoseContainer::empty() const
{
    return
        m_doses.empty();
}

void DoseContainer::addDose(
    const DoseGridModel& dose)
{
    m_doses.push_back(dose);

    if (m_currentIndex == -1)
        m_currentIndex = 0;
}

void DoseContainer::removeDose(
    int index)
{
    if (index < 0 ||
        index >= size())
        return;

    m_doses.erase(
        m_doses.begin() + index);

    if (m_doses.empty())
    {
        m_currentIndex = -1;
    }
    else if (m_currentIndex >= size())
    {
        m_currentIndex = size() - 1;
    }
}

DoseGridModel&
DoseContainer::current()
{
    return
        m_doses[m_currentIndex];
}

const DoseGridModel&
DoseContainer::current() const
{
    return
        m_doses[m_currentIndex];
}

DoseGridModel*
DoseContainer::get(
    int index)
{
    if (index < 0 ||
        index >= size())
        return nullptr;

    return
        &m_doses[index];
}

const DoseGridModel*
DoseContainer::get(
    int index) const
{
    if (index < 0 ||
        index >= size())
        return nullptr;

    return
        &m_doses[index];
}

void DoseContainer::setCurrentIndex(
    int index)
{
    if (index < 0 ||
        index >= size())
        return;

    m_currentIndex = index;
}

int DoseContainer::currentIndex() const
{
    return
        m_currentIndex;
}