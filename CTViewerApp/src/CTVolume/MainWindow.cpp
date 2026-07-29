#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include "SliceViewer.h"
#include "RTSTRUCT/RTStructLoader.h"
#include "RTSTRUCT/StructureSetContainer.h"
#include "RTSTRUCT/StructureListWidget.h"
#include "RTSTRUCT/ContourVoxelizer.h"
#include "RTSTRUCT/StructureVolumeCalculator.h"
#include "RTDOSE/RTDoseLoader.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();

    progressBar->setTextVisible(true);
    progressBar->setFormat("%p%");
    
    qApp->setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #dddddd;
        }

        QLineEdit, QTreeWidget {
            background-color: #3c3c3c;
            border: 1px solid #555;
        }

        QPushButton {
            background-color: #444;
            border: 1px solid #666;
            padding: 5px;
        }

        QPushButton:hover {
            background-color: #555;
        }

        QProgressBar {
            border: 1px solid #555;
            text-align: center;
            background-color: #3c3c3c;
        }

        QProgressBar::chunk {
            background-color: #3daee9;
        }
    )");
}
void MainWindow::setupUI()
{
    QWidget* central = new QWidget();
    setCentralWidget(central);

    QVBoxLayout* layout = new QVBoxLayout(central);

    // ===== Folder row =====
    QHBoxLayout* topLayout = new QHBoxLayout();

    txtFolder = new QLineEdit();
    btnBrowse = new QPushButton("Browse");

    topLayout->addWidget(txtFolder);
    topLayout->addWidget(btnBrowse);

    // ===== Tree =====
    treeSeries = new QTreeWidget();
    treeSeries->setHeaderLabel("DICOM Series");

    // ===== Progress =====
    progressBar = new QProgressBar();
    progressBar->setValue(0);

    // ===== Bottom =====
    QHBoxLayout* bottomLayout = new QHBoxLayout();

    status = new QLabel("Ready");
    btnImport = new QPushButton("Import");

    bottomLayout->addWidget(status);
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnImport);

    // ===== Add =====
    layout->addLayout(topLayout);
    layout->addWidget(treeSeries);
    layout->addWidget(progressBar);
    layout->addLayout(bottomLayout);

    // ===== Connect =====
    connect(btnBrowse, &QPushButton::clicked, this, &MainWindow::onBrowse);
    connect(btnImport, &QPushButton::clicked, this, &MainWindow::onImport);

    setWindowTitle("DICOM RT Import");
    resize(600, 500);
}
void MainWindow::onBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select DICOM Folder");

    if (dir.isEmpty()) return;

    txtFolder->setText(dir);

    scanFolder(dir);
}
void MainWindow::scanFolder(const QString& path)
{
    patients.clear();
    treeSeries->clear();

    progressBar->setValue(0);
    progressBar->setMaximum(100);
    status->setText("Scanning...");

    CTLoader loader;

    loader.scanFolder(
        path.toStdString(),
        patients,
        [this](int current, int total, const std::string&)
        {
            if (total > 0)
            {
                int percent = (current * 100) / total;
                progressBar->setValue(percent);
            }

            QApplication::processEvents(); 
        });

    progressBar->setValue(100); 

    populateTree();

    status->setText("Loaded OK");
}
void MainWindow::populateTree()
{
    for (auto& [pName, patient] : patients)
    {
        QTreeWidgetItem* patientItem = new QTreeWidgetItem(treeSeries);
        patientItem->setText(0, QString::fromStdString(pName));

        for (auto& [sUID, series] : patient.seriesMap)
        {
            QString modality = QString::fromStdString(series.modality);

            /*if (modality.compare("CT", Qt::CaseInsensitive) != 0) //display CT only
                continue; */

            QString text = QString("%1 | %2 slices | %3")
                .arg(modality)
                .arg(series.slices.size())
                .arg(QString::fromStdString(series.description));

            QTreeWidgetItem* seriesItem = new QTreeWidgetItem(patientItem);
            seriesItem->setText(0, text);

            // checkbox
            seriesItem->setCheckState(0, Qt::Unchecked);

            // lưu UID
            seriesItem->setData(0, Qt::UserRole, QString::fromStdString(sUID));
        }
    }

    treeSeries->expandAll();
}
void MainWindow::onImport()
{
    importQueue.clear();

    hasRTStruct = false;

    currentImportIndex = 0;

    // ==========================================
    // READ CHECKED ITEMS
    // ==========================================

    for (int i = 0; i < treeSeries->topLevelItemCount(); i++)
    {
        auto patientItem =
            treeSeries->topLevelItem(i);

        for (int j = 0; j < patientItem->childCount(); j++)
        {
            auto seriesItem =
                patientItem->child(j);

            if (seriesItem->checkState(0)
                != Qt::Checked)
            {
                continue;
            }

            QString uid =
                seriesItem->data(
                    0,
                    Qt::UserRole).toString();

            std::string pName =
                patientItem->text(0).toStdString();

            auto& series =
                patients[pName]
                .seriesMap[uid.toStdString()];

            QString modality =
                QString::fromStdString(
                    series.modality);

            // =====================================
            // CT / MR
            // =====================================

            if (modality == "CT" ||
                modality == "MR")
            {
                importQueue.push_back(series);
            }

            // =====================================
            // RTSTRUCT
            // =====================================

            else if (modality == "RTSTRUCT")
            {
                hasRTStruct = true;

                selectedRTStructSeries = series;
            }

			// =====================================
			// RTDOSE
			// =====================================
            else if (modality == "RTDOSE")
            {
                hasRTDose = true;

                selectedRTDoseSeries = series;
            }
        }
    }

    // ==========================================
    // START IMPORT
    // ==========================================

    processNextSeries();
}
void MainWindow::processNextSeries()
{
    if (currentImportIndex >= importQueue.size())
        return;

    CTLoader loader;
    CTVolume volume;

    auto& series = importQueue[currentImportIndex];
    loader.buildVolume(series, volume);

    // ===== SHOW METADATA =====
    QString info;

    info += "Patient: " + QString::fromStdString(series.patientName) + "\n";
    info += "Patient ID: " + QString::fromStdString(series.patientID) + "\n";
    info += "Study: " + QString::fromStdString(series.studyDescription) + "\n";

    info += "Modality: " + QString::fromStdString(series.modality) + "\n";
    info += "Series: " + QString::fromStdString(series.description) + "\n";
    info += "Series Number: " + QString::fromStdString(series.seriesNumber) + "\n";

    // ===== SIZE =====
    info += "Rows: " + QString::number(volume.height) + "\n";
    info += "Columns: " + QString::number(volume.width) + "\n";
    info += "Slices: " + QString::number(volume.depth) + "\n";

    // ===== SPACING =====
    info += "Pixel Spacing (X, Y): " +
        QString("%1, %2")
        .arg(volume.spacing[0], 0, 'f', 3)
        .arg(volume.spacing[1], 0, 'f', 3) + "\n";

    info += "Slice Thickness (Z): " +
        QString::number(volume.spacing[2]) + "\n";

    // ===== ORIGIN + Position =====

    info += "Image Position (Patient): " +
        QString("(%1, %2, %3)")
        .arg(volume.origin[0])
        .arg(volume.origin[1])
        .arg(volume.origin[2]) + "\n";

    // ===== ORIENTATION =====
    info += "Image Orientation (Patient): " +
        QString("(%1, %2, %3, %4, %5, %6)")
        .arg(volume.direction[0])
        .arg(volume.direction[1])
        .arg(volume.direction[2])
        .arg(volume.direction[3])
        .arg(volume.direction[4])
        .arg(volume.direction[5]) + "\n";

    // ===== RESCALE =====
    info += "Rescale Slope: " +
        QString::number(volume.rescaleSlope) + "\n";

    info += "Rescale Intercept: " +
        QString::number(volume.rescaleIntercept) + "\n";

    // ===== SHOW DIALOG =====
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("CT Metadata");
    msgBox->setText(info);
    msgBox->setStandardButtons(QMessageBox::Ok);

    // Khi user bấm OK → mở viewer
    connect(msgBox, &QMessageBox::buttonClicked, this,
        [this, volume, series](QAbstractButton*)
        {
            this->hide();

            // =====================================
            // CREATE VIEWER
            // =====================================

            SliceViewer* viewer =
                new SliceViewer();

            viewer->setVolume(volume);

            viewer->setDoseGrid(
                &doseGrid);

            viewer->resize(700, 700);

            viewer->setAttribute(
                Qt::WA_DeleteOnClose);


            //=====================================
            // LOAD RTDOSE
            //=====================================
    
            if (hasRTDose)
            {
                RTDoseLoader doseLoader;

                qDebug() << "---------------------------------------";
                qDebug() << "Loading RTDOSE...";
                qDebug() << "File:"
                    << QString::fromStdString(
                        selectedRTDoseSeries.slices[0].path);

                if (doseLoader.loadDose(
                    selectedRTDoseSeries.slices[0].path,
                    doseGrid))
                {
                    viewer->setDoseGrid(&doseGrid);

                    qDebug() << "RTDOSE loaded successfully.";

                    qDebug() << "Grid Size :"
                        << doseGrid.width
                        << "x"
                        << doseGrid.height
                        << "x"
                        << doseGrid.depth;

                    qDebug() << "Spacing :"
                        << doseGrid.spacing[0]
                        << doseGrid.spacing[1]
                        << doseGrid.spacing[2];

                    qDebug() << "Origin :"
                        << doseGrid.origin[0]
                        << doseGrid.origin[1]
                        << doseGrid.origin[2];

                    qDebug() << "Dose Unit :"
                        << QString::fromStdString(
                            doseGrid.doseUnit);

                    qDebug() << "Dose Type :"
                        << QString::fromStdString(
                            doseGrid.doseType);

                    qDebug() << "Dose Summation :"
                        << QString::fromStdString(
                            doseGrid.doseSummationType);

                    qDebug() << "Dose Grid Scaling :"
                        << doseGrid.doseGridScaling;

                    qDebug() << "Minimum Dose (Gy):"
                        << doseGrid.getMinimumDose();

                    qDebug() << "Maximum Dose (Gy):"
                        << doseGrid.getMaximumDose();

                    qDebug() << "Voxel Count:"
                        << doseGrid.dose.size();
                }
                else
                {
                    qDebug() << "Failed to load RTDOSE.";
                }

                qDebug() << "---------------------------------------";
            }


            // =====================================
            // LOAD RTSTRUCT
            // =====================================

            if (hasRTStruct &&
                series.modality == "CT")
            {
                qDebug()
                    << "========== LOAD RTSTRUCT ==========";

                qDebug()
                    << "CT FOR:"
                    << QString::fromStdString(
                        volume.frameOfReferenceUID);

                qDebug()
                    << "RTSTRUCT FOR:"
                    << QString::fromStdString(
                        selectedRTStructSeries.frameOfReferenceUID);

                auto* structureSet =
                    new StructureSetContainer();

                bool ok =
                    RTStructLoader::load(
                        selectedRTStructSeries
                        .slices[0]
                        .path,
                        &volume,
                        *structureSet);
                qDebug()
                    << "Structure count:"
                    << structureSet->structures.size();

                if (ok)
                {
                    qDebug()
                        << "RTSTRUCT LOAD SUCCESS";

                    // =====================================
                    // CREATE MASK + CALCULATE VOLUME
                    // =====================================

                    for (auto& structure : structureSet->structures)
                    {
                        structure.mask =
                            ContourVoxelizer::createMask(
                                structure,
                                volume);

                        structure.volumeCM3 =
                            StructureVolumeCalculator::
                            computeVolumeCM3(
                                structure.mask,
                                volume);

                        qDebug()
                            << QString::fromStdString(structure.name)
                            << structure.volumeCM3
                            << "cm3";

                        double vol =
                            StructureVolumeCalculator::
                            computeVolumeCM3(
                                structure.mask,
                                volume);

                        qDebug()
                            << QString::fromStdString(
                                structure.name)
                            << "Volume ="
                            << vol
                            << "cm3";
                    }

                    // =====================================
                    // SHOW IN VIEWER
                    // =====================================

                    viewer->setStructureSet(
                        structureSet);
                }
                else
                {
                    qDebug()
                        << "RTSTRUCT LOAD FAILED";

                    delete structureSet;

                    QMessageBox::warning(
                        this,
                        "RTSTRUCT",
                        "RTSTRUCT does not match CT");
                }
            }

            // =====================================
            // CLOSE EVENT
            // =====================================

            connect(viewer,
                &QObject::destroyed,
                this,
                [this]()
                {
                    this->show();

                    currentImportIndex++;

                    processNextSeries();
                });

            viewer->showMaximized();
        });

    msgBox->show();
}