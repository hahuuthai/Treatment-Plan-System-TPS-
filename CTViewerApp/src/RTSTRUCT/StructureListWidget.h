#pragma once

#include <QWidget>
#include <QTreeWidget>

#include "StructureModel.h"

class StructureListWidget : public QWidget
{
    Q_OBJECT

public:

    explicit StructureListWidget(
        QWidget* parent = nullptr);

    void setStructures(
        std::vector<StructureModel>* structures);

    std::vector<int> getSelectedStructureIndices() const;

signals:

    void visibilityChanged();
    void colorClicked(int structureIndex);

private:

    QTreeWidget* tree;

    std::vector<StructureModel>* m_structures = nullptr;

    void updateChildren(
        QTreeWidgetItem* parent,
        Qt::CheckState state);

    void updateParentState(
        QTreeWidgetItem* item);
};