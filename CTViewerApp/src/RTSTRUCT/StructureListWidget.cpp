#include "StructureListWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>

StructureListWidget::StructureListWidget(
    QWidget* parent)
    : QWidget(parent)
{
    tree = new QTreeWidget;

    tree->setSelectionMode(
        QAbstractItemView::ExtendedSelection);

    tree->setSelectionBehavior(
        QAbstractItemView::SelectRows);

    tree->setColumnCount(5);

    tree->setHeaderLabels(
        {
            "Show",
            " ",
			"Color",
            "Type",
			"Volume (cc)"
        });

    tree->header()->setSectionResizeMode(
        0,
        QHeaderView::ResizeToContents);

    tree->header()->setSectionResizeMode(
        2,
        QHeaderView::ResizeToContents);

    tree->header()->setSectionResizeMode(
        3,
        QHeaderView::ResizeToContents);

    tree->header()->setSectionResizeMode(
        4,
        QHeaderView::ResizeToContents);

    tree->header()->setSectionResizeMode(
        1,
        QHeaderView::Interactive);

    auto* layout =
        new QVBoxLayout(this);

    layout->addWidget(tree);

    connect(
        tree,
        &QTreeWidget::itemChanged,
        this,
        [this](QTreeWidgetItem* item, int)
        {
            if (!m_structures)
                return;

            tree->blockSignals(true);

            Qt::CheckState state =
                item->checkState(0);

            //-------------------------------------------------
            // update children
            //-------------------------------------------------

            updateChildren(item, state);

            //-------------------------------------------------
            // update parent
            //-------------------------------------------------

            updateParentState(item);

            //-------------------------------------------------
            // update structure visible
            //-------------------------------------------------

            QVariant idxVar =
                item->data(0, Qt::UserRole);

            if (idxVar.isValid())
            {
                int idx = idxVar.toInt();

                if (idx >= 0 &&
                    idx < m_structures->size())
                {
                    (*m_structures)[idx].visible =
                        state == Qt::Checked;
                }
            }

            tree->blockSignals(false);

            emit visibilityChanged();
        });
}

void StructureListWidget::setStructures(
    std::vector<StructureModel>* structures)
{
    m_structures = structures;

    tree->clear();

    if (!m_structures)
        return;

    //-------------------------------------------------
    // Root
    //-------------------------------------------------

    auto* root =
        new QTreeWidgetItem(tree);

    root->setText(
        1,
        "Display All");

    root->setCheckState(
        0,
        Qt::Checked);

    //-------------------------------------------------
    // map type -> group item
    //-------------------------------------------------

    std::map<
        QString,
        QTreeWidgetItem*> groups;

    //-------------------------------------------------
    // create groups
    //-------------------------------------------------

    for (int i = 0;
        i < m_structures->size();
        i++)
    {
        const auto& s =
            (*m_structures)[i];

        QString type =
            QString::fromStdString(
                s.type);

        QPushButton* colorBtn =
            new QPushButton();

        colorBtn->setFixedSize(20, 20);

        colorBtn->setStyleSheet(
            QString(
                "background-color:%1;"
                "border:1px solid white;")
            .arg(s.color.name()));

        connect(
            colorBtn,
            &QPushButton::clicked,
            this,
            [this, i]()
            {
                emit colorClicked(i);
            });

        if (!groups.count(type))
        {
            auto* group =
                new QTreeWidgetItem(
                    root);

            group->setText(
                1,
                type);

            group->setCheckState(
                0,
                Qt::Checked);

            groups[type] =
                group;
        }

        //-------------------------------------------------
        // leaf
        //-------------------------------------------------

        auto* item =
            new QTreeWidgetItem(
                groups[type]);

        item->setCheckState(
            0,
            s.visible
            ? Qt::Checked
            : Qt::Unchecked);

        item->setText(
            1,
            QString::fromStdString(
                s.name));

        item->setData(
            0,
            Qt::UserRole,
            i);

        tree->setItemWidget(
            item,
            2,
            colorBtn);

        item->setText(
            3,
            QString::fromStdString(
                s.type));

        item->setText(
            4,
            QString::number(
                s.volumeCM3,
                'f',
                2));
    }

    tree->expandAll();
}

void StructureListWidget::updateChildren(
    QTreeWidgetItem* parent,
    Qt::CheckState state)
{
    for (int i = 0;
        i < parent->childCount();
        i++)
    {
        auto* child =
            parent->child(i);

        child->setCheckState(
            0,
            state);

        updateChildren(
            child,
            state);

        QVariant idx =
            child->data(
                0,
                Qt::UserRole);

        if (idx.isValid() &&
            m_structures)
        {
            int id =
                idx.toInt();

            (*m_structures)[id].visible =
                state ==
                Qt::Checked;
        }
    }
}

void StructureListWidget::updateParentState(
    QTreeWidgetItem* item)
{
    auto* parent =
        item->parent();

    if (!parent)
        return;

    int checked = 0;
    int partial = 0;

    for (int i = 0;
        i < parent->childCount();
        i++)
    {
        auto s =
            parent->child(i)
            ->checkState(0);

        if (s == Qt::Checked)
            checked++;

        else if (s ==
            Qt::PartiallyChecked)
            partial++;
    }

    if (checked ==
        parent->childCount())
    {
        parent->setCheckState(
            0,
            Qt::Checked);
    }
    else if (checked == 0 &&
        partial == 0)
    {
        parent->setCheckState(
            0,
            Qt::Unchecked);
    }
    else
    {
        parent->setCheckState(
            0,
            Qt::PartiallyChecked);
    }

    updateParentState(parent);
}

std::vector<int>
StructureListWidget::getSelectedStructureIndices() const
{
    std::vector<int> result;

    auto items =
        tree->selectedItems();

    for (auto* item : items)
    {
        QVariant v =
            item->data(
                0,
                Qt::UserRole);

        if (v.isValid())
        {
            result.push_back(
                v.toInt());
        }
    }

    return result;
}