#include "WindowLevelController.h"
#include <QSignalBlocker>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QEvent>
#include <algorithm>
#include <cmath>

WindowLevelController::WindowLevelController(QWidget* parent)
    : QObject(parent)
{
    // ===== WINDOW / LEVEL BOX =====
    wlBox = new QGroupBox("Window (HU)");
    wlBox->setFixedWidth(260);

    widthSpin = new QSpinBox;
    widthSpin->setRange(1, 4096);
    widthSpin->setValue(400);
    widthSpin->setSingleStep(2);

    levelSpin = new QSpinBox;
    levelSpin->setRange(-3000, 3000);
    levelSpin->setValue(40);
    levelSpin->setSingleStep(1);

    QLabel* widthLabel = new QLabel("Width ");
    QLabel* levelLabel = new QLabel("Level");
    widthLabel->setFixedWidth(50);
    levelLabel->setFixedWidth(50);

    auto* wlLayout = new QGridLayout;
    wlLayout->setContentsMargins(5, 5, 5, 5);
    wlLayout->setHorizontalSpacing(5);
    wlLayout->addWidget(widthLabel, 0, 0);
    wlLayout->addWidget(widthSpin, 0, 1);
    wlLayout->addWidget(levelLabel, 1, 0);
    wlLayout->addWidget(levelSpin, 1, 1);
    wlBox->setLayout(wlLayout);

    // ===== LIMITS BOX =====
    limitBox = new QGroupBox("Limits (HU)");
    limitBox->setFixedWidth(260);

    minSpin = new QSpinBox;
    minSpin->setRange(-1000, 3071);
    minSpin->setValue((int)m_minLimit);

    maxSpin = new QSpinBox;
    maxSpin->setRange(-1000, 3071);
    maxSpin->setValue((int)m_maxLimit);

    QLabel* minLabel = new QLabel("Min");
    QLabel* maxLabel = new QLabel("Max");
    minLabel->setFixedWidth(50);
    maxLabel->setFixedWidth(50);

    auto* limitLayout = new QGridLayout;
    limitLayout->setContentsMargins(5, 5, 5, 5);
    limitLayout->setHorizontalSpacing(10);
    limitLayout->addWidget(minLabel, 0, 0);
    limitLayout->addWidget(minSpin, 0, 1);
    limitLayout->addWidget(maxLabel, 1, 0);
    limitLayout->addWidget(maxSpin, 1, 1);
    limitBox->setLayout(limitLayout);

    widthSpin->installEventFilter(this);
    levelSpin->installEventFilter(this);
    minSpin->installEventFilter(this);
    maxSpin->installEventFilter(this);

    // ===== CONNECT: MIN LIMIT =====
    connect(minSpin, &QSpinBox::editingFinished, this, [this]()
        {
            if (minSpin->value() == prevMin) return;

            m_minLimit = minSpin->value();
            m_maxLimit = maxSpin->value();
            recomputeFromLimits();
        });

    // ===== CONNECT: MAX LIMIT =====
    connect(maxSpin, &QSpinBox::editingFinished, this, [this]()
        {
            int newMax = maxSpin->value();
            int currentMin = minSpin->value();

            if (newMax < currentMin)
            {
                QMessageBox::warning(
                    qobject_cast<QWidget*>(this->QObject::parent()),
                    "Invalid Input",
                    "Max must be greater than or equal to Min");

                QSignalBlocker b(maxSpin);
                maxSpin->setValue(currentMin);
                return;
            }

            if (newMax == prevMax) return;

            m_minLimit = currentMin;
            m_maxLimit = newMax;
            recomputeFromLimits();
        });

    // ===== CONNECT: WINDOW WIDTH =====
    connect(widthSpin, &QSpinBox::editingFinished, this, [this]()
        {
            int val = widthSpin->value();
            if (val == prevWidth) return;

            m_window = val;
            m_level = levelSpin->value();
            recomputeFromWindowLevel();
        });

    // ===== CONNECT: WINDOW LEVEL =====
    connect(levelSpin, &QSpinBox::editingFinished, this, [this]()
        {
            int val = levelSpin->value();
            if (val == prevLevel) return;

            m_level = val;
            m_window = widthSpin->value();
            recomputeFromWindowLevel();
        });
}

bool WindowLevelController::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FocusIn)
    {
        if (obj == minSpin)   prevMin = minSpin->value();
        if (obj == maxSpin)   prevMax = maxSpin->value();
        if (obj == widthSpin) prevWidth = widthSpin->value();
        if (obj == levelSpin) prevLevel = levelSpin->value();
    }

    return QObject::eventFilter(obj, event);
}

int WindowLevelController::calcLevel(double minL, double maxL) const
{
    double sum = maxL + minL;
    double lv = (std::fmod(sum, 2.0) == 0.0) ? sum / 2.0 : std::ceil(sum / 2.0);
    return (int)std::lround(lv);
}

void WindowLevelController::recomputeFromLimits()
{
    m_window = std::max(0.0, m_maxLimit - m_minLimit) + 1.0;
    m_level = calcLevel(m_minLimit, m_maxLimit);

    {
        QSignalBlocker b1(widthSpin);
        QSignalBlocker b2(levelSpin);
        widthSpin->setValue((int)std::lround(m_window));
        levelSpin->setValue((int)m_level);
    }

    prevWidth = widthSpin->value();
    prevLevel = (int)m_level;
    prevMin = (int)m_minLimit;
    prevMax = (int)m_maxLimit;

    emit limitsChanged();
}

void WindowLevelController::recomputeFromWindowLevel()
{
    m_minLimit = std::floor(m_level - (m_window - 1.0) / 2.0);
    m_maxLimit = std::floor(m_level + (m_window - 1.0) / 2.0);

    {
        QSignalBlocker b1(minSpin);
        QSignalBlocker b2(maxSpin);
        minSpin->setValue((int)m_minLimit);
        maxSpin->setValue((int)m_maxLimit);
    }

    prevMin = (int)m_minLimit;
    prevMax = (int)m_maxLimit;

    emit limitsChanged();
}

void WindowLevelController::setFromVolumeRange(float minVoxel, float maxVoxel)
{
    m_minLimit = minVoxel;
    m_maxLimit = maxVoxel;

    m_window = std::max(0.0, m_maxLimit - m_minLimit) + 1.0;
    m_level = calcLevel(m_minLimit, m_maxLimit);

    QSignalBlocker b1(widthSpin);
    QSignalBlocker b2(levelSpin);
    QSignalBlocker b3(minSpin);
    QSignalBlocker b4(maxSpin);

    widthSpin->setValue((int)std::lround(m_window));
    levelSpin->setValue((int)m_level);
    minSpin->setValue((int)m_minLimit);
    maxSpin->setValue((int)m_maxLimit);

    prevWidth = widthSpin->value();
    prevLevel = levelSpin->value();
    prevMin = (int)m_minLimit;
    prevMax = (int)m_maxLimit;
}

void WindowLevelController::applyDrag(int dx, int dy)
{
    int dW = dx * 2;
    int dL = -dy;

    int newWW = qBound(1, widthSpin->value() + dW, 100000);
    int newWL = qBound(-3000, levelSpin->value() + dL, 3000);

    m_window = newWW;
    m_level = newWL;

    {
        QSignalBlocker b1(widthSpin);
        QSignalBlocker b2(levelSpin);
        widthSpin->setValue(newWW);
        levelSpin->setValue(newWL);
    }

    m_minLimit = std::floor(m_level - (m_window - 1.0) / 2.0);
    m_maxLimit = std::floor(m_level + (m_window - 1.0) / 2.0);

    {
        QSignalBlocker b3(minSpin);
        QSignalBlocker b4(maxSpin);
        minSpin->setValue((int)m_minLimit);
        maxSpin->setValue((int)m_maxLimit);
    }

    prevWidth = newWW;
    prevLevel = newWL;
    prevMin = (int)m_minLimit;
    prevMax = (int)m_maxLimit;

    emit limitsChanged();
}