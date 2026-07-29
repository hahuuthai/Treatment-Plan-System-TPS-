#include "SliceViewer.h"
#include "SliceRenderer.h"
#include "StructureOperationPanel.h"
#include "StructureMarginPanel.h"
#include "RTDOSE/DoseSampler.h"

#include <algorithm>
#include <cmath>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QVBoxLayout>
#include <QColorDialog>

SliceViewer::SliceViewer(QWidget* parent)
    : QMainWindow(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    // ===== IMAGE LABEL / SCROLL AREA =====
    imageLabel = new QLabel();
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(false);
    imageLabel->installEventFilter(this);
    imageLabel->setMouseTracking(true);
    setMouseTracking(true);

    scrollArea = new QScrollArea();
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { background-color: #202020; }");
    scrollArea->setMinimumSize(1200, 700);
    scrollArea->setMaximumSize(1200, 700);

    // ===== STATUS BAR =====
    infoLabel = new QLabel("HU: ");
    infoLabel->setFixedHeight(30);
    infoLabel->setStyleSheet(
        "QLabel { color: white; background-color: black; padding-left: 10px;"
        "font-family: Consolas; font-size: 12px; }");

    // ===== WINDOW / LEVEL =====
    wlController = new WindowLevelController(this);
    connect(wlController, &WindowLevelController::limitsChanged, this, [this]()
        {
            updateSlice(currentSlice);
        });

    // ===== LAYOUT =====
    auto* central = new QWidget();
    setCentralWidget(central);

    auto* viewerLayout = new QVBoxLayout();
    viewerLayout->setContentsMargins(0, 0, 0, 0);
    viewerLayout->setSpacing(0);
    viewerLayout->addWidget(scrollArea, 1, Qt::AlignCenter);
    viewerLayout->addWidget(wlController->widthLevelBox());
    viewerLayout->addWidget(wlController->limitsBox());
    viewerLayout->addWidget(infoLabel);
    central->setLayout(viewerLayout);

    // ===== LEFT TOOLBAR =====
    leftToolbar = new QToolBar(this);
    leftToolbar->setOrientation(Qt::Vertical);
    leftToolbar->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, leftToolbar);
    leftToolbar->setFixedWidth(90);
    leftToolbar->setStyleSheet(
        "QToolBar { background:#2d2d30; spacing:5px; }"
        "QToolButton { min-width:80px; min-height:40px; }");
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

    volumeAction = leftToolbar->addAction("Volumes");
    operationAction = leftToolbar->addAction("Operations");
    marginAction = leftToolbar->addAction("Create Margin");

    // ===== VOLUME PANEL =====
    volumePanel = new QWidget(this);
    volumePanel->setFixedWidth(320);
    volumePanel->setAttribute(Qt::WA_StyledBackground, true);
    volumePanel->setStyleSheet(
        "background-color:#2d2d30; border-right:1px solid #555;");

    structureWidget = new StructureListWidget(volumePanel);

    connect(structureWidget, &StructureListWidget::visibilityChanged, this, [this]()
        {
            updateSlice(currentSlice);
        });

    connect(structureWidget, &StructureListWidget::colorClicked,
        this, &SliceViewer::changeStructureColor);

    auto* panelLayout = new QVBoxLayout(volumePanel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->addWidget(structureWidget);

    volumePanel->setGeometry(leftToolbar->width(), 0, 350, 550);
    volumePanel->hide();

    // ===== OPERATION PANEL =====
    operationPanel = new StructureOperationPanel(this);
    operationPanel->setStructureListWidget(structureWidget);
    operationPanel->setGeometry(leftToolbar->width(), 50, 350, 550);
    operationPanel->hide();

    connect(operationPanel, &StructureOperationPanel::structuresModified,
        this, &SliceViewer::refreshAfterStructuresModified);

    // ===== MARGIN PANEL =====
    marginPanel = new StructureMarginPanel(this);
    marginPanel->setGeometry(leftToolbar->width(), 100, 350, 550);
    marginPanel->hide();

    connect(marginPanel, &StructureMarginPanel::structuresModified,
        this, &SliceViewer::refreshAfterStructuresModified);

    // ===== TOGGLE PANEL =====
    connect(volumeAction, &QAction::triggered, this, [this]()
        {
            volumePanel->setVisible(!volumePanel->isVisible());
            operationPanel->hide();
            marginPanel->hide();
        });

    connect(operationAction, &QAction::triggered, this, [this]()
        {
            operationPanel->setVisible(!operationPanel->isVisible());
            volumePanel->hide();
            marginPanel->hide();
        });

    connect(marginAction, &QAction::triggered, this, [this]()
        {
            marginPanel->setVisible(!marginPanel->isVisible());
            volumePanel->hide();
            operationPanel->hide();
        });

    zoom = 1.0f;
}

void SliceViewer::setVolume(const CTVolume& vol)
{
    volume = vol;
    currentSlice = 0;
    zoom = 1.0f;
    panOffsetX = 0;
    panOffsetY = 0;

    if (!volume.voxels.empty())
    {
        auto [minIt, maxIt] =
            std::minmax_element(volume.voxels.begin(), volume.voxels.end());

        wlController->setFromVolumeRange(*minIt, *maxIt);
    }

    operationPanel->setVolume(&volume);
    marginPanel->setVolume(&volume);

    updateSlice(currentSlice);
}

void SliceViewer::setStructureSet(StructureSetContainer* set)
{
    structureSet = set;

    operationPanel->setStructureSet(set);
    marginPanel->setStructureSet(set);

    if (!structureWidget || !set)
    {
        updateSlice(currentSlice);
        return;
    }

    structureWidget->setStructures(&set->structures);

    operationPanel->refreshVolumeList();
    marginPanel->refreshVolumeList();

    updateSlice(currentSlice);
}

void SliceViewer::refreshAfterStructuresModified()
{
    if (!structureSet) return;

    structureWidget->setStructures(&structureSet->structures);
    operationPanel->refreshVolumeList();
    marginPanel->refreshVolumeList();

    updateSlice(currentSlice);
}

void SliceViewer::changeStructureColor(int structureIndex)
{
    if (!structureSet) return;

    QColor current = structureSet->structures[structureIndex].color;
    QColor newColor = QColorDialog::getColor(current, this, "Select Color");
    if (!newColor.isValid()) return;

    structureSet->structures[structureIndex].color = newColor;

    structureWidget->setStructures(&structureSet->structures);
    updateSlice(currentSlice);
}

void SliceViewer::updateSlice(int z)
{
    if (volume.voxels.empty()) return;

    currentSlice = z;

    QImage img = SliceRenderer::renderSlice(
        volume, z, wlController->minLimit(), wlController->maxLimit(), structureSet);

    currentPixmap = QPixmap::fromImage(img);

    QPixmap scaledPix = currentPixmap.scaled(
        static_cast<int>(currentPixmap.width() * zoom),
        static_cast<int>(currentPixmap.height() * zoom),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    // ===== CANVAS =====
    QPixmap canvas(scrollArea->viewport()->width(), scrollArea->viewport()->height());
    canvas.fill(Qt::black);

    QPainter canvasPainter(&canvas);

    int x = (canvas.width() - scaledPix.width()) / 2 + panOffsetX;
    int y = (canvas.height() - scaledPix.height()) / 2 + panOffsetY;

    canvasPainter.drawPixmap(x, y, scaledPix);

    // ===== ORIENTATION LABELS =====
    canvasPainter.setRenderHint(QPainter::TextAntialiasing, true);

    QFont oriFont("Arial", 28, QFont::Bold);
    canvasPainter.setFont(oriFont);
    canvasPainter.setPen(Qt::yellow);

    std::array<double, 3> row =
    { volume.direction[0], volume.direction[1], volume.direction[2] };
    std::array<double, 3> col =
    { volume.direction[3], volume.direction[4], volume.direction[5] };

    QString rightLabel = SliceRenderer::getOrientationLabel(row);
    QString leftLabel = SliceRenderer::getOrientationLabel({ -row[0], -row[1], -row[2] });
    QString downLabel = SliceRenderer::getOrientationLabel(col);
    QString upLabel = SliceRenderer::getOrientationLabel({ -col[0], -col[1], -col[2] });

    int viewW = canvas.width();
    int viewH = canvas.height();
    const int margin = 5;

    canvasPainter.drawText(
        QRect(0, margin, viewW, 50), Qt::AlignHCenter | Qt::AlignTop, upLabel);

    canvasPainter.drawText(
        QRect(0, viewH - 50 - margin, viewW, 50), Qt::AlignHCenter | Qt::AlignBottom, downLabel);

    canvasPainter.drawText(
        QRect(margin, 0, 80, viewH), Qt::AlignLeft | Qt::AlignVCenter, leftLabel);

    canvasPainter.drawText(
        QRect(viewW - 80 - margin, 0, 80, viewH), Qt::AlignRight | Qt::AlignVCenter, rightLabel);

    imageLabel->setPixmap(canvas);
    imageLabel->adjustSize();

    if (hasImagePos)
        updateInfo(lastImageX, lastImageY, currentSlice);
    else
        updateInfo(volume.width / 2, volume.height / 2, currentSlice);
}

void SliceViewer::updateInfo(int x, int y, int z)
{
    if (volume.voxels.empty()) return;
    if (x < 0 || y < 0 || x >= volume.width || y >= volume.height) return;

    int idx = z * volume.width * volume.height + y * volume.width + x;
    float val = volume.voxels[idx];

    QString label =
        (volume.modality == "CT")
        ? "HU"
        : "Intensity";

    if (volume.modality == "CT" && val < -1024.0f)
        val = -1000.0f;

    
    auto pos3D =
        CoordinateAlignment::ctIndexToPatient(
            volume,
            x,
            y,
            z);

    QString text =
        QString("Slice: %1/%2")
        .arg(z + 1)
        .arg(volume.depth);

    text +=
        QString(" | Image Position (Patient, at cursor): (%1, %2, %3)")
        .arg(pos3D[0], 0, 'f', 2)
        .arg(pos3D[1], 0, 'f', 2)
        .arg(pos3D[2], 0, 'f', 2);

    text +=
        QString(" | %1: %2")
        .arg(label)
        .arg(val, 0, 'f', 1);


    //--------------------------------------------------
    // Dose
    //--------------------------------------------------

    if (doseGrid && doseGrid->isValid())
    {
        qDebug() << "Dose Grid Size :"
            << doseGrid->width
            << doseGrid->height
            << doseGrid->depth;

        //--------------------------------------------------
        // CT -> Dose Index
        //--------------------------------------------------

        auto doseIndex =
            CoordinateAlignment::ctIndexToDoseIndex(
                volume,
                *doseGrid,
                x,
                y,
                z);

        qDebug() << "Mapped Dose Index :"
            << doseIndex[0]
            << doseIndex[1]
            << doseIndex[2];

        bool inside =
            doseGrid->contains(
                doseIndex[0],
                doseIndex[1],
                doseIndex[2]);

        qDebug() << "Inside Dose Grid :" << inside;

        //--------------------------------------------------
        // Sample
        //--------------------------------------------------

        float doseValue =
            DoseSampler::sampleCT(
                volume,
                *doseGrid,
                x,
                y,
                z,
                DoseSampler::SamplingMode::Trilinear);

        qDebug() << "Sampled Dose :" << doseValue << "Gy";
        if (doseGrid->doseUnit == "GY")
        {
            double doseValue_cGy =
                double(doseValue) * 100.0;

            text +=
                QString(" | Dose: %1 cGy")
                .arg(doseValue_cGy, 0, 'f', 2);
        }
        else
        {
            text +=
                QString(" | Dose: %1 %2")
                .arg(doseValue, 0, 'f', 4)
                .arg(QString::fromStdString(doseGrid->doseUnit));
        }
    }
    else
    {
        qDebug() << "Dose Grid is NULL";
    }

    infoLabel->setText(text);
}

void SliceViewer::keyPressEvent(QKeyEvent* event)
{
    if (volume.voxels.empty()) return;

    if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Up)
        currentSlice++;
    else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Down)
        currentSlice--;
    else
    {
        QWidget::keyPressEvent(event);
        return;
    }

    currentSlice = std::clamp(currentSlice, 0, volume.depth - 1);
    updateSlice(currentSlice);
}

bool SliceViewer::eventFilter(QObject* obj, QEvent* event)
{
    if (obj != imageLabel)
        return QMainWindow::eventFilter(obj, event);

    // ===== MOUSE PRESS =====
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto* e = static_cast<QMouseEvent*>(event);

        if (e->button() == Qt::LeftButton)
        {
            setFocus();

            if (QApplication::keyboardModifiers() & Qt::ControlModifier)
            {
                isPanning = true;
                lastPanPos = e->pos();
            }
            else
            {
                isAdjustingWL = true;
                lastWLPos = e->pos();
            }
        }
        return true;
    }

    // ===== MOUSE RELEASE =====
    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto* e = static_cast<QMouseEvent*>(event);

        if (e->button() == Qt::LeftButton)
        {
            isAdjustingWL = false;
            isPanning = false;
        }
        return true;
    }

    // ===== MOUSE MOVE =====
    if (event->type() == QEvent::MouseMove)
    {
        auto* e = static_cast<QMouseEvent*>(event);
        lastMousePos = e->pos();
        hasMousePos = true;

        if (isPanning)
        {
            QPoint delta = e->pos() - lastPanPos;
            lastPanPos = e->pos();

            panOffsetX += delta.x();
            panOffsetY += delta.y();

            updateSlice(currentSlice);
            return true;
        }

        if (isAdjustingWL)
        {
            QPoint delta = e->pos() - lastWLPos;
            lastWLPos = e->pos();

            wlController->applyDrag(delta.x(), delta.y());
        }
        else
        {
            if (volume.voxels.empty()) return true;

            QPixmap pix = imageLabel->pixmap();
            if (pix.isNull()) return true;

            int scaledW = static_cast<int>(volume.width * zoom);
            int scaledH = static_cast<int>(volume.height * zoom);

            int offsetX = (pix.width() - scaledW) / 2;
            int offsetY = (pix.height() - scaledH) / 2;

            int imgX = e->pos().x() - offsetX;
            int imgY = e->pos().y() - offsetY;

            if (imgX < 0 || imgY < 0 || imgX >= scaledW || imgY >= scaledH)
                return true;

            int x = static_cast<int>(imgX / zoom);
            int y = static_cast<int>(imgY / zoom);

            if (x >= 0 && y >= 0 && x < volume.width && y < volume.height)
            {
                lastImageX = x;
                lastImageY = y;
                hasImagePos = true;

                updateInfo(x, y, currentSlice);
            }
        }

        return true;
    }

    // ===== WHEEL =====
    if (event->type() == QEvent::Wheel)
    {
        auto* wheelEvent = static_cast<QWheelEvent*>(event);

        if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        {
            int delta = wheelEvent->angleDelta().y();
            zoom *= (delta > 0) ? 1.1f : (1.0f / 1.1f);
            zoom = std::clamp(zoom, 0.2f, 5.0f);

            updateSlice(currentSlice);
            return true;
        }

        int delta = wheelEvent->angleDelta().y();
        currentSlice += (delta > 0) ? 1 : -1;
        currentSlice = std::clamp(currentSlice, 0, volume.depth - 1);

        updateSlice(currentSlice);
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}

void SliceViewer::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (volumePanel)
        volumePanel->setGeometry(leftToolbar->width(), 0, 350, 550);

    if (operationPanel)
        operationPanel->setGeometry(leftToolbar->width(), 50, 350, 550);

    if (marginPanel)
        marginPanel->setGeometry(leftToolbar->width(), 100, 350, 550);
}

void SliceViewer::setDoseGrid(
    DoseGridModel* dose)
{
   
    DoseSampler::clearCache();

    doseGrid = dose;
}