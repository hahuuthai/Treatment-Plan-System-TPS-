#pragma once
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include "CTVolume.h"

class StructureSetContainer;

// Panel "Create Margin": mở rộng/thu hẹp 1 structure theo margin (symmetric),
// có tùy chọn trừ đi vùng tránh (Avoidance).
class StructureMarginPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StructureMarginPanel(QWidget* parent = nullptr);

    void setStructureSet(StructureSetContainer* set);
    void setVolume(const CTVolume* volume);

    void refreshVolumeList();
    void resetPanel();

signals:
    void structuresModified();

public slots:
    void executeMargin();

private slots:
    void updateOkButtonState();
    void updateAvoidVolumeList();

private:
    StructureSetContainer* structureSet = nullptr;
    const CTVolume* volume = nullptr;

    QComboBox* marginSourceCombo = nullptr;

    QCheckBox* symmetricCheck = nullptr;
    QDoubleSpinBox* marginAllSpin = nullptr;

    QDoubleSpinBox* superiorSpin = nullptr;
    QDoubleSpinBox* inferiorSpin = nullptr;
    QDoubleSpinBox* leftSpin = nullptr;
    QDoubleSpinBox* rightSpin = nullptr;
    QDoubleSpinBox* anteriorSpin = nullptr;
    QDoubleSpinBox* posteriorSpin = nullptr;

    QComboBox* avoidVolumeCombo = nullptr;
    QDoubleSpinBox* avoidMarginSpin = nullptr;

    QPushButton* marginOkButton = nullptr;
    QPushButton* marginCancelButton = nullptr;

    QRadioButton* marginCreateNewRadio = nullptr;
    QRadioButton* marginApplyRadio = nullptr;
    QLineEdit* marginResultNameEdit = nullptr;
    QComboBox* marginResultTypeCombo = nullptr;
    QComboBox* marginApplyVolumeCombo = nullptr;
};
