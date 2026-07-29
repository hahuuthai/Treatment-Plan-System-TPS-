#pragma once
#include <QObject>
#include <QSpinBox>
#include <QGroupBox>

// Đóng gói toàn bộ UI + logic Window/Level và Limits (HU).
// SliceViewer chỉ cần add widthLevelBox()/limitsBox() vào layout và lắng nghe limitsChanged().
class WindowLevelController : public QObject
{
    Q_OBJECT

public:
    explicit WindowLevelController(QWidget* parent = nullptr);

    QWidget* widthLevelBox() const { return wlBox; }
    QWidget* limitsBox() const { return limitBox; }

    // Gọi khi load volume mới (set min/max theo dữ liệu voxel thực tế)
    void setFromVolumeRange(float minVoxel, float maxVoxel);

    // Gọi từ SliceViewer::eventFilter khi kéo chuột trái để chỉnh WW/WL
    void applyDrag(int dx, int dy);

    double minLimit() const { return m_minLimit; }
    double maxLimit() const { return m_maxLimit; }
    double window() const { return m_window; }
    double level() const { return m_level; }

signals:
    void limitsChanged();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    int  calcLevel(double minL, double maxL) const;
    void recomputeFromLimits();
    void recomputeFromWindowLevel();

    QGroupBox* wlBox = nullptr;
    QGroupBox* limitBox = nullptr;

    QSpinBox* widthSpin = nullptr;
    QSpinBox* levelSpin = nullptr;
    QSpinBox* minSpin = nullptr;
    QSpinBox* maxSpin = nullptr;

    double m_window = 400.0;
    double m_level = 40.0;
    double m_minLimit = -1000.0;
    double m_maxLimit = 3000.0;

    int prevMin = -1000;
    int prevMax = 3000;
    int prevWidth = 400;
    int prevLevel = 40;
};
