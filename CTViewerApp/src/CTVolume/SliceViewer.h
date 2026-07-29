#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QToolBar>
#include <QAction>
#include <QPixmap>
#include "CTVolume.h"
#include "RTSTRUCT/StructureSetContainer.h"
#include "RTSTRUCT/StructureListWidget.h"
#include "WindowLevelController.h"
#include "RTDOSE/DoseGridModel.h"

class StructureOperationPanel;
class StructureMarginPanel;

class SliceViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit SliceViewer(QWidget* parent = nullptr);
    void setVolume(const CTVolume& vol);

    StructureSetContainer* structureSet = nullptr;
    void setStructureSet(StructureSetContainer* structureSet);

    void setDoseGrid(
        DoseGridModel* dose);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void changeStructureColor(int structureIndex);
    void refreshAfterStructuresModified();

private:
    void updateSlice(int z);
    void updateInfo(int x, int y, int z);

    // ===== SLICE / ZOOM =====
    int currentSlice = 0;
    float zoom = 1.0f;

    QPoint lastMousePos;
    bool hasMousePos = false;

    // ===== UI CORE =====
    QLabel* imageLabel = nullptr;
    QLabel* infoLabel = nullptr;
    QScrollArea* scrollArea = nullptr;

    QPixmap currentPixmap;
    CTVolume volume;

    // ===== WINDOW / LEVEL =====
    WindowLevelController* wlController = nullptr;
    bool isAdjustingWL = false;
    QPoint lastWLPos;

    // ===== PANNING =====
    bool isPanning = false;
    QPoint lastPanPos;
    int panOffsetX = 0;
    int panOffsetY = 0;

    // ===== LAST IMAGE POS =====
    int lastImageX = 0;
    int lastImageY = 0;
    bool hasImagePos = false;

    // ===== LEFT TOOLBAR & PANELS =====
    QToolBar* leftToolbar = nullptr;

    QAction* volumeAction = nullptr;
    QAction* operationAction = nullptr;
    QAction* marginAction = nullptr;

    QWidget* volumePanel = nullptr;
    StructureListWidget* structureWidget = nullptr;

    StructureOperationPanel* operationPanel = nullptr;
    StructureMarginPanel* marginPanel = nullptr;

	// ===== RTDOSE =====
    DoseGridModel* doseGrid = nullptr;
};
