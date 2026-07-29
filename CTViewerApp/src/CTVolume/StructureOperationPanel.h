#pragma once
#include <QWidget>
#include <QComboBox>
#include <QTableWidget>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <functional>
#include "CTVolume.h"
#include "RTSTRUCT/StructureModel.h"
#include "RTSTRUCT/MaskVolume.h"

class StructureSetContainer;
class StructureListWidget;

// Panel "Operations": Union/Intersection/Difference giữa 2 structure có sẵn,
// cộng thêm các quick-action (Union/Subtract/Intersect/Margin 5mm nhanh dựa trên
// lựa chọn hiện tại trong StructureListWidget).
class StructureOperationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StructureOperationPanel(QWidget* parent = nullptr);

    void setStructureSet(StructureSetContainer* set);
    void setVolume(const CTVolume* volume);
    void setStructureListWidget(StructureListWidget* widget);

    // Nạp lại danh sách volume vào các combobox (gọi sau khi structureSet thay đổi)
    void refreshVolumeList();
    void resetPanel();

signals:
    // Phát ra khi structureSet bị thay đổi (thêm/sửa structure) để SliceViewer refresh UI
    void structuresModified();

public slots:
    void executeOperation();

    // Quick actions dùng lựa chọn hiện tại trong StructureListWidget
    void createUnion();
    void createSubtract();
    void createIntersection();
    void createMargin();

private slots:
    void updateOkButtonState();
    void updateVolumeList();

private:
    void runBooleanQuickAction(
        const QString& name,
        std::function<MaskVolume(const MaskVolume&, const MaskVolume&)> op,
        const QString& warnTitle);

    StructureSetContainer* structureSet = nullptr;
    const CTVolume* volume = nullptr;
    StructureListWidget* structureListWidget = nullptr;

    QComboBox* firstVolumeCombo = nullptr;
    QTableWidget* operationTable = nullptr;

    QRadioButton* createNewRadio = nullptr;
    QRadioButton* applyRadio = nullptr;
    QLineEdit* resultNameEdit = nullptr;
    QComboBox* resultTypeCombo = nullptr;
    QComboBox* applyVolumeCombo = nullptr;

    QPushButton* okButton = nullptr;
    QPushButton* cancelButton = nullptr;
};
