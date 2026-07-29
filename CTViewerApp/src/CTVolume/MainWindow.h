#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QLabel>
#include <QProgressBar>

#include "CTLoader.h"
#include "RTDOSE/DoseGridModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private:

    QLineEdit* txtFolder;
    QPushButton* btnBrowse;
    QTreeWidget* treeSeries;
    QPushButton* btnImport;
    QLabel* status;
    QProgressBar* progressBar;

    std::map<std::string, Patient> patients;

    // =========================
    // IMPORT QUEUE
    // =========================
    std::vector<Series> importQueue;

    int currentImportIndex = 0;

    // =========================
    // RTSTRUCT
    // =========================
    bool hasRTStruct = false;

    Series selectedRTStructSeries;

    // =========================
    // RTDOSE
    // =========================

    bool hasRTDose = false;

    Series selectedRTDoseSeries;

    DoseGridModel doseGrid;


private:

    void setupUI();

    void scanFolder(
        const QString& path);

    void populateTree();

    void processNextSeries();

private slots:

    void onBrowse();

    void onImport();
};