#include "StructureOperationPanel.h"
#include "RTSTRUCT/StructureSetContainer.h"
#include "RTSTRUCT/StructureListWidget.h"
#include "RTSTRUCT/StructureBoolean.h"
#include "RTSTRUCT/StructureMarginExpansion.h"
#include "RTSTRUCT/StructureVolumeCalculator.h"
#include "ColorPalette.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>

StructureOperationPanel::StructureOperationPanel(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(420);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "background-color:#2d2d30;"
        "border-right:1px solid #555;");

    auto* opLayout = new QVBoxLayout(this);

    QLabel* firstLabel = new QLabel("First volume");
    firstVolumeCombo = new QComboBox();

    opLayout->addWidget(firstLabel);
    opLayout->addWidget(firstVolumeCombo);

    operationTable = new QTableWidget(1, 3);
    operationTable->setHorizontalHeaderLabels(
        { "Operation", "Volume Name", "Margin (cm)" });

    QComboBox* opCombo = new QComboBox();
    opCombo->addItems({ "Union", "Intersection", "Difference" });
    operationTable->setCellWidget(0, 0, opCombo);

    QComboBox* volumeCombo = new QComboBox();
    operationTable->setCellWidget(0, 1, volumeCombo);

    QDoubleSpinBox* marginSpin = new QDoubleSpinBox();
    marginSpin->setRange(-10.0, 10.0);
    marginSpin->setSingleStep(0.5);
    marginSpin->setDecimals(1);
    marginSpin->setValue(0.0);
    operationTable->setCellWidget(0, 2, marginSpin);

    opLayout->addWidget(operationTable);

    QStringList structureTypes =
    {
        "PTV", "CTV", "GTV", "Treated Volume",
        "Irradiated Volume", "Organ", "Isodose", "OAR"
    };

    QGroupBox* resultGroup = new QGroupBox("Results");
    auto* resultLayout = new QVBoxLayout(resultGroup);

    createNewRadio = new QRadioButton("Create New Volume");
    applyRadio = new QRadioButton("Apply to Volume");
    createNewRadio->setChecked(true);

    resultNameEdit = new QLineEdit();
    resultTypeCombo = new QComboBox();
    resultTypeCombo->addItems(structureTypes);
    applyVolumeCombo = new QComboBox();

    resultLayout->addWidget(createNewRadio);
    resultLayout->addWidget(resultNameEdit);
    resultLayout->addWidget(resultTypeCombo);
    resultLayout->addWidget(applyRadio);
    resultLayout->addWidget(applyVolumeCombo);

    connect(resultNameEdit, &QLineEdit::textChanged,
        this, &StructureOperationPanel::updateOkButtonState);
    connect(createNewRadio, &QRadioButton::toggled,
        this, &StructureOperationPanel::updateOkButtonState);
    connect(applyRadio, &QRadioButton::toggled,
        this, &StructureOperationPanel::updateOkButtonState);

    opLayout->addWidget(resultGroup);

    auto* bottomLayout = new QHBoxLayout();
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");

    const QString btnStyle =
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
        "}";

    okButton->setStyleSheet(btnStyle);
    cancelButton->setStyleSheet(btnStyle);

    connect(okButton, &QPushButton::clicked,
        this, &StructureOperationPanel::executeOperation);

    connect(cancelButton, &QPushButton::clicked, this, [this]()
        {
            resetPanel();
            hide();
        });

    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);
    opLayout->addLayout(bottomLayout);
    opLayout->addStretch();

    connect(firstVolumeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &StructureOperationPanel::updateVolumeList);

    updateOkButtonState();
}

void StructureOperationPanel::setStructureSet(StructureSetContainer* set)
{
    structureSet = set;
}

void StructureOperationPanel::setVolume(const CTVolume* v)
{
    volume = v;
}

void StructureOperationPanel::setStructureListWidget(StructureListWidget* w)
{
    structureListWidget = w;
}

void StructureOperationPanel::refreshVolumeList()
{
    if (!structureSet) return;

    firstVolumeCombo->clear();
    applyVolumeCombo->clear();

    if (auto* second = qobject_cast<QComboBox*>(operationTable->cellWidget(0, 1)))
        second->clear();

    for (const auto& s : structureSet->structures)
    {
        QString name = QString::fromStdString(s.name);
        firstVolumeCombo->addItem(name);
        applyVolumeCombo->addItem(name);
    }

    updateVolumeList();
}

void StructureOperationPanel::updateVolumeList()
{
    if (!structureSet) return;

    auto* second = qobject_cast<QComboBox*>(operationTable->cellWidget(0, 1));
    if (!second) return;

    int first = firstVolumeCombo->currentIndex();

    second->blockSignals(true);
    second->clear();

    for (int i = 0; i < static_cast<int>(structureSet->structures.size()); ++i)
    {
        if (i == first) continue;

        second->addItem(
            QString::fromStdString(structureSet->structures[i].name), i);
    }

    if (second->count() > 0)
        second->setCurrentIndex(0);

    second->blockSignals(false);
}

void StructureOperationPanel::updateOkButtonState()
{
    if (applyRadio->isChecked())
    {
        okButton->setEnabled(true);
        resultNameEdit->setStyleSheet("");
        return;
    }

    bool valid = !resultNameEdit->text().trimmed().isEmpty();
    okButton->setEnabled(valid);

    resultNameEdit->setStyleSheet(
        valid ? "" : "QLineEdit { border:2px solid red; }");
}

void StructureOperationPanel::resetPanel()
{
    firstVolumeCombo->setCurrentIndex(0);

    if (auto* opCombo = qobject_cast<QComboBox*>(operationTable->cellWidget(0, 0)))
        opCombo->setCurrentIndex(0);

    if (auto* volCombo = qobject_cast<QComboBox*>(operationTable->cellWidget(0, 1)))
        volCombo->setCurrentIndex(0);

    createNewRadio->setChecked(true);
    resultNameEdit->clear();
    resultTypeCombo->setCurrentIndex(0);
    applyVolumeCombo->setCurrentIndex(0);

    updateOkButtonState();
}

void StructureOperationPanel::executeOperation()
{
    if (!structureSet || !volume) return;

    if (createNewRadio->isChecked())
    {
        QString name = resultNameEdit->text().trimmed();

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

    int firstIndex = firstVolumeCombo->currentIndex();

    auto* opCombo = qobject_cast<QComboBox*>(operationTable->cellWidget(0, 0));
    auto* secondCombo = qobject_cast<QComboBox*>(operationTable->cellWidget(0, 1));
    auto* marginSpin = qobject_cast<QDoubleSpinBox*>(operationTable->cellWidget(0, 2));

    if (!marginSpin || !opCombo || !secondCombo) return;

    double marginMM = marginSpin->value() * 10.0;
    int secondIndex = secondCombo->currentData().toInt();

    if (firstIndex < 0 || secondIndex < 0) return;

    QString op = opCombo->currentText();

    MaskVolume firstMask = structureSet->structures[firstIndex].mask;
    MaskVolume secondMask = structureSet->structures[secondIndex].mask;

    if (marginMM > 0.0)
        secondMask = StructureMarginExpansion::expand(secondMask, *volume, marginMM);

    MaskVolume result;

    if (op == "Union")
        result = StructureBoolean::Union(firstMask, secondMask);
    else if (op == "Intersection")
        result = StructureBoolean::Intersect(firstMask, secondMask);
    else if (op == "Difference")
        result = StructureBoolean::Subtract(firstMask, secondMask);

    if (createNewRadio->isChecked())
    {
        StructureModel s;
        s.name = resultNameEdit->text().toStdString();
        s.type = resultTypeCombo->currentText().toStdString();
        s.color = ColorPalette::generateUniqueColor(structureSet);
        s.mask = std::move(result);
        s.volumeCM3 = StructureVolumeCalculator::computeVolumeCM3(s.mask, *volume);

        structureSet->structures.push_back(std::move(s));
    }
    else
    {
        int dst = applyVolumeCombo->currentIndex();

        if (dst >= 0)
        {
            auto& dstStruct = structureSet->structures[dst];
            dstStruct.mask = std::move(result);
            dstStruct.contours.clear();
            dstStruct.volumeCM3 =
                StructureVolumeCalculator::computeVolumeCM3(dstStruct.mask, *volume);
        }
    }

    emit structuresModified();

    QMessageBox::information(this, "Operation", "Operation completed successfully.");

    resetPanel();
    hide();
}

void StructureOperationPanel::runBooleanQuickAction(
    const QString& name,
    std::function<MaskVolume(const MaskVolume&, const MaskVolume&)> op,
    const QString& warnTitle)
{
    if (!structureSet || !volume || !structureListWidget) return;

    auto ids = structureListWidget->getSelectedStructureIndices();

    if (ids.size() != 2)
    {
        QMessageBox::warning(this, warnTitle, "Select exactly 2 structures.");
        return;
    }

    int id1 = ids[0];
    int id2 = ids[1];

    okButton->setEnabled(false);
    cancelButton->setEnabled(false);

    auto* watcher = new QFutureWatcher<StructureModel>(this);

    auto future = QtConcurrent::run(
        [this, id1, id2, name, op]() -> StructureModel
        {
            StructureModel s;
            s.name = name.toStdString();
            s.type = "Derived";
            s.color = ColorPalette::generateUniqueColor(structureSet);
            s.mask = op(
                structureSet->structures[id1].mask,
                structureSet->structures[id2].mask);
            s.volumeCM3 = StructureVolumeCalculator::computeVolumeCM3(s.mask, *volume);
            return s;
        });

    connect(watcher, &QFutureWatcher<StructureModel>::finished, this, [=]()
        {
            StructureModel s = watcher->result();
            structureSet->structures.push_back(s);

            emit structuresModified();

            okButton->setEnabled(true);
            cancelButton->setEnabled(true);

            watcher->deleteLater();
        });

    watcher->setFuture(future);
}

void StructureOperationPanel::createUnion()
{
    runBooleanQuickAction("Union", &StructureBoolean::Union, "Union");
}

void StructureOperationPanel::createSubtract()
{
    runBooleanQuickAction("Subtract", &StructureBoolean::Subtract, "Subtract");
}

void StructureOperationPanel::createIntersection()
{
    runBooleanQuickAction("Intersection", &StructureBoolean::Intersect, "Intersection");
}

void StructureOperationPanel::createMargin()
{
    if (!structureSet || !volume || !structureListWidget) return;

    auto ids = structureListWidget->getSelectedStructureIndices();

    if (ids.size() != 1)
    {
        QMessageBox::warning(this, "Margin", "Select exactly 1 structure.");
        return;
    }

    int id = ids[0];

    okButton->setEnabled(false);
    cancelButton->setEnabled(false);

    auto* watcher = new QFutureWatcher<StructureModel>(this);

    auto future = QtConcurrent::run(
        [this, id]() -> StructureModel
        {
            StructureModel s;
            s.name = "Margin5mm";
            s.type = "Derived";
            s.mask = StructureMarginExpansion::expand(
                structureSet->structures[id].mask, *volume, 5.0);
            s.volumeCM3 = StructureVolumeCalculator::computeVolumeCM3(s.mask, *volume);
            return s;
        });

    connect(watcher, &QFutureWatcher<StructureModel>::finished, this, [=]()
        {
            StructureModel s = watcher->result();
            structureSet->structures.push_back(s);

            emit structuresModified();

            okButton->setEnabled(true);
            cancelButton->setEnabled(true);

            watcher->deleteLater();
        });

    watcher->setFuture(future);
}