#include "StructureMarginPanel.h"
#include "RTSTRUCT/StructureSetContainer.h"
#include "RTSTRUCT/StructureModel.h"
#include "RTSTRUCT/StructureBoolean.h"
#include "RTSTRUCT/StructureMarginExpansion.h"
#include "RTSTRUCT/StructureVolumeCalculator.h"
#include "ColorPalette.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>

StructureMarginPanel::StructureMarginPanel(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(450);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "background-color:#2d2d30;"
        "border-right:1px solid #555;");

    auto* marginLayout = new QVBoxLayout(this);

    QLabel* srcLabel = new QLabel("Original Volume");
    marginSourceCombo = new QComboBox();

    QGroupBox* marginBox = new QGroupBox("Margin (cm)");
    auto* marginGrid = new QGridLayout(marginBox);

    symmetricCheck = new QCheckBox("Symmetric");
    symmetricCheck->setChecked(true);

    marginAllSpin = new QDoubleSpinBox();
    marginAllSpin->setRange(-10.0, 10.0);
    marginAllSpin->setSingleStep(0.5);
    marginAllSpin->setDecimals(1);
    marginAllSpin->setValue(0.5);

    marginGrid->addWidget(symmetricCheck, 0, 0);
    marginGrid->addWidget(marginAllSpin, 0, 1);

    QStringList dirs =
    {
        "Superior", "Inferior", "Left", "Right", "Anterior", "Posterior"
    };

    superiorSpin = new QDoubleSpinBox();
    inferiorSpin = new QDoubleSpinBox();
    leftSpin = new QDoubleSpinBox();
    rightSpin = new QDoubleSpinBox();
    anteriorSpin = new QDoubleSpinBox();
    posteriorSpin = new QDoubleSpinBox();

    marginAllSpin->setEnabled(true);
    superiorSpin->setEnabled(false);
    inferiorSpin->setEnabled(false);
    leftSpin->setEnabled(false);
    rightSpin->setEnabled(false);
    anteriorSpin->setEnabled(false);
    posteriorSpin->setEnabled(false);

    QDoubleSpinBox* spins[] =
    {
        superiorSpin, inferiorSpin, leftSpin, rightSpin, anteriorSpin, posteriorSpin
    };

    for (int i = 0; i < 6; i++)
    {
        spins[i]->setRange(-10.0, 10.0);
        spins[i]->setSingleStep(0.5);
        spins[i]->setDecimals(1);
        spins[i]->setValue(0.5);

        marginGrid->addWidget(new QLabel(dirs[i]), i + 1, 0);
        marginGrid->addWidget(spins[i], i + 1, 1);
    }

    QGroupBox* avoidBox = new QGroupBox("Avoidance");
    auto* avoidLayout = new QVBoxLayout(avoidBox);

    avoidVolumeCombo = new QComboBox();
    avoidMarginSpin = new QDoubleSpinBox();
    avoidMarginSpin->setRange(-10.0, 10.0);
    avoidMarginSpin->setSingleStep(0.5);
    avoidMarginSpin->setDecimals(1);
    avoidMarginSpin->setValue(1.0);

    avoidLayout->addWidget(avoidVolumeCombo);
    avoidLayout->addWidget(avoidMarginSpin);

    connect(symmetricCheck, &QCheckBox::toggled, this, [=](bool checked)
        {
            marginAllSpin->setEnabled(checked);

            superiorSpin->setEnabled(!checked);
            inferiorSpin->setEnabled(!checked);
            leftSpin->setEnabled(!checked);
            rightSpin->setEnabled(!checked);
            anteriorSpin->setEnabled(!checked);
            posteriorSpin->setEnabled(!checked);

            if (checked)
            {
                double v = marginAllSpin->value();
                superiorSpin->setValue(v);
                inferiorSpin->setValue(v);
                leftSpin->setValue(v);
                rightSpin->setValue(v);
                anteriorSpin->setValue(v);
                posteriorSpin->setValue(v);
            }
        });

    connect(marginAllSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, [=](double value)
        {
            if (!symmetricCheck->isChecked())
                return;

            superiorSpin->setValue(value);
            inferiorSpin->setValue(value);
            leftSpin->setValue(value);
            rightSpin->setValue(value);
            anteriorSpin->setValue(value);
            posteriorSpin->setValue(value);
        });

    // ===== RESULTS =====
    QGroupBox* marginResultGroup = new QGroupBox("Results");
    auto* marginResultLayout = new QVBoxLayout(marginResultGroup);

    marginCreateNewRadio = new QRadioButton("Create New Volume");
    marginApplyRadio = new QRadioButton("Apply To Volume");
    marginCreateNewRadio->setChecked(true);

    marginResultNameEdit = new QLineEdit();
    marginResultNameEdit->setText("Margin");

    marginResultTypeCombo = new QComboBox();
    marginResultTypeCombo->addItems(
        { "PTV", "CTV", "GTV", "Treated Volume",
          "Irradiated Volume", "Organ", "Isodose", "OAR" });

    marginApplyVolumeCombo = new QComboBox();

    marginResultLayout->addWidget(marginCreateNewRadio);
    marginResultLayout->addWidget(marginResultNameEdit);
    marginResultLayout->addWidget(marginResultTypeCombo);
    marginResultLayout->addWidget(marginApplyRadio);
    marginResultLayout->addWidget(marginApplyVolumeCombo);

    connect(marginResultNameEdit, &QLineEdit::textChanged,
        this, &StructureMarginPanel::updateOkButtonState);
    connect(marginCreateNewRadio, &QRadioButton::toggled,
        this, &StructureMarginPanel::updateOkButtonState);
    connect(marginApplyRadio, &QRadioButton::toggled,
        this, &StructureMarginPanel::updateOkButtonState);

    // ===== OK / CANCEL =====
    marginOkButton = new QPushButton("OK");
    marginCancelButton = new QPushButton("Cancel");

    marginOkButton->setStyleSheet(
        "QPushButton {"
        " background-color:#2d2d30;"
        " color:white;"
        " border:1px solid #666;"
        "}"
        "QPushButton:hover {"
        " background-color:#0078D7;"
        "}"
        "QPushButton:disabled {"
        " background-color:#3a3a3a;"
        " color:#777;"
        "}");

    marginCancelButton->setStyleSheet(
        "QPushButton {"
        " background-color:#2d2d30;"
        " color:white;"
        " border:1px solid #666;"
        "}"
        "QPushButton:hover {"
        " background-color:#D83B01;"
        "}"
        "QPushButton:disabled {"
        " background-color:#3a3a3a;"
        " color:#777;"
        "}");

    connect(marginOkButton, &QPushButton::clicked,
        this, &StructureMarginPanel::executeMargin);

    connect(marginCancelButton, &QPushButton::clicked, this, [this]()
        {
            resetPanel();
            hide();
        });

    auto* marginBottomLayout = new QHBoxLayout();
    marginBottomLayout->addWidget(marginOkButton);
    marginBottomLayout->addWidget(marginCancelButton);

    marginLayout->addWidget(srcLabel);
    marginLayout->addWidget(marginSourceCombo);
    marginLayout->addWidget(marginBox);
    marginLayout->addWidget(avoidBox);
    marginLayout->addWidget(marginResultGroup);
    marginLayout->addLayout(marginBottomLayout);
    marginLayout->addStretch();

    connect(marginSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &StructureMarginPanel::updateAvoidVolumeList);

    updateOkButtonState();
}

void StructureMarginPanel::setStructureSet(StructureSetContainer* set)
{
    structureSet = set;
}

void StructureMarginPanel::setVolume(const CTVolume* v)
{
    volume = v;
}

void StructureMarginPanel::refreshVolumeList()
{
    if (!structureSet) return;

    marginSourceCombo->clear();
    marginApplyVolumeCombo->clear();

    for (const auto& s : structureSet->structures)
    {
        QString name = QString::fromStdString(s.name);
        marginSourceCombo->addItem(name);
        marginApplyVolumeCombo->addItem(name);
    }

    updateAvoidVolumeList();
}

void StructureMarginPanel::updateAvoidVolumeList()
{
    if (!structureSet) return;

    int src = marginSourceCombo->currentIndex();

    avoidVolumeCombo->blockSignals(true);
    avoidVolumeCombo->clear();
    avoidVolumeCombo->addItem("None", -1);

    for (int i = 0; i < static_cast<int>(structureSet->structures.size()); ++i)
    {
        if (i == src) continue;

        avoidVolumeCombo->addItem(
            QString::fromStdString(structureSet->structures[i].name), i);
    }

    avoidVolumeCombo->setCurrentIndex(0);
    avoidVolumeCombo->blockSignals(false);
}

void StructureMarginPanel::updateOkButtonState()
{
    if (marginApplyRadio->isChecked())
    {
        marginOkButton->setEnabled(true);
        marginResultNameEdit->setStyleSheet("");
        return;
    }

    bool valid = !marginResultNameEdit->text().trimmed().isEmpty();
    marginOkButton->setEnabled(valid);

    marginResultNameEdit->setStyleSheet(
        valid ? "" : "QLineEdit { border:2px solid red; }");
}

void StructureMarginPanel::resetPanel()
{
    marginSourceCombo->setCurrentIndex(0);

    symmetricCheck->setChecked(true);
    marginAllSpin->setValue(0.5);

    superiorSpin->setValue(0.5);
    inferiorSpin->setValue(0.5);
    leftSpin->setValue(0.5);
    rightSpin->setValue(0.5);
    anteriorSpin->setValue(0.5);
    posteriorSpin->setValue(0.5);

    avoidVolumeCombo->setCurrentIndex(0);
    avoidMarginSpin->setValue(1.0);

    marginCreateNewRadio->setChecked(true);
    marginResultNameEdit->clear();
    marginResultTypeCombo->setCurrentIndex(0);
    marginApplyVolumeCombo->setCurrentIndex(0);

    updateOkButtonState();
}

void StructureMarginPanel::executeMargin()
{
    if (!structureSet || !volume) return;

    int src = marginSourceCombo->currentIndex();
    if (src < 0) return;

    double marginMM = 0.0;

    if (symmetricCheck->isChecked())
    {
        marginMM = marginAllSpin->value() * 10.0; // cm -> mm
    }
    else
    {
        QMessageBox::warning(this, "Margin", "Asymmetric margin is not implemented yet.");
        return;
    }

    const StructureModel& source = structureSet->structures[src];

    if (source.mask.voxels.empty())
    {
        QMessageBox::warning(this, "Margin", "Selected structure has no mask.");
        return;
    }

    bool createNew = marginCreateNewRadio->isChecked();
    QString resultName = marginResultNameEdit->text();

    if (createNew)
    {
        QString name = resultName.trimmed();

        if (name.isEmpty())
        {
            QMessageBox::warning(this, "Invalid Name", "Volume name cannot be empty.");
            return;
        }

        for (const auto& s : structureSet->structures)
        {
            if (QString::fromStdString(s.name).compare(name, Qt::CaseInsensitive) == 0)
            {
                QMessageBox::warning(this, "Duplicate Name",
                    "A volume with this name already exists.");
                return;
            }
        }
    }

    marginOkButton->setEnabled(false);
    marginCancelButton->setEnabled(false);

    QString resultType = marginResultTypeCombo->currentText();
    int dst = marginApplyVolumeCombo->currentIndex();
    int avoidIndex = avoidVolumeCombo->currentData().toInt();
    double avoidMarginMM = avoidMarginSpin->value() * 10.0;

    auto* watcher = new QFutureWatcher<StructureModel>(this);

    auto future = QtConcurrent::run(
        [this, src, marginMM, avoidIndex, avoidMarginMM]() -> StructureModel
        {
            StructureModel resultStruct;

            MaskVolume result = StructureMarginExpansion::expand(
                structureSet->structures[src].mask, *volume, marginMM);

            if (avoidIndex >= 0 && avoidIndex != src)
            {
                MaskVolume avoidMask = StructureMarginExpansion::expand(
                    structureSet->structures[avoidIndex].mask, *volume, avoidMarginMM);

                result = StructureBoolean::Subtract(result, avoidMask);
            }

            resultStruct.mask = std::move(result);
            resultStruct.volumeCM3 =
                StructureVolumeCalculator::computeVolumeCM3(resultStruct.mask, *volume);

            return resultStruct;
        });

    connect(watcher, &QFutureWatcher<StructureModel>::finished, this,
        [=]()
        {
            StructureModel resultStruct = watcher->result();

            if (createNew)
            {
                resultStruct.name = resultName.toStdString();
                resultStruct.type = resultType.toStdString();
                resultStruct.color = ColorPalette::generateUniqueColor(structureSet);

                structureSet->structures.push_back(std::move(resultStruct));
            }
            else
            {
                auto& dstStruct = structureSet->structures[dst];
                dstStruct.mask = std::move(resultStruct.mask);
                dstStruct.contours.clear();
                dstStruct.volumeCM3 = resultStruct.volumeCM3;
            }

            emit structuresModified();

            marginOkButton->setEnabled(true);
            marginCancelButton->setEnabled(true);

            QMessageBox::information(this, "Margin", "Margin created successfully.");

            hide();
            watcher->deleteLater();
        });

    watcher->setFuture(future);

    resetPanel();
    hide();
}