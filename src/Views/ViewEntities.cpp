#include <windows.h>

#include "Configuration.h"
#include "Data/Entity.h"
#include "Data/EntityDB.h"
#include "Data/State.h"
#include "Data/StdMap.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewEntities.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QTimer>

S2Plugin::ViewEntities::ViewEntities(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    auto config = Configuration::get();
    mLayer0Offset = config->offsetForField(MemoryFieldType::State, "layer0", Spelunky2::get()->get_StatePtr());
    mLayer1Offset = config->offsetForField(MemoryFieldType::State, "layer1", Spelunky2::get()->get_StatePtr());
    mLayerMapOffset = config->offsetForField(config->typeFieldsOfDefaultStruct("LayerPointer"), "entities_by_mask");

    initializeRefreshAndFilter();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle("Entities");
    mMainTreeView->setVisible(true);

    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColMemoryAddressDelta).disable(gsColMemoryAddress).disable(gsColComment);
    refreshEntities();
    mFilterLineEdit->setFocus();
}

void S2Plugin::ViewEntities::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->setEnableChangeHighlighting(false);

    mMainLayout->addWidget(mMainTreeView);
}

void S2Plugin::ViewEntities::initializeRefreshAndFilter()
{
    auto filterLayout = new QGridLayout(this);

    auto refreshButton = new QPushButton("Refresh", this);
    QObject::connect(refreshButton, &QPushButton::clicked, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(refreshButton, 0, 0);

    auto label = new QLabel("Filter:", this);
    filterLayout->addWidget(label, 0, 1);

    mFilterLineEdit = new QLineEdit(this);
    mFilterLineEdit->setPlaceholderText("Search for UID (dec or hex starting with 0x) or (part of) the entity name");
    QObject::connect(mFilterLineEdit, &QLineEdit::textChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mFilterLineEdit, 0, 2, 1, 6);

    mCheckboxLayer0 = new QCheckBox("Front layer (0)", this);
    mCheckboxLayer0->setChecked(true);
    QObject::connect(mCheckboxLayer0, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxLayer0, 2, 2);

    mCheckboxLayer1 = new QCheckBox("Back layer (0)", this);
    QObject::connect(mCheckboxLayer1, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxLayer1, 2, 3);

    int row = 3;
    int col = 2;
    for (auto& checkbox : mCheckbox)
    {
        checkbox.mCheckbox = new QCheckBox(checkbox.name + " (0)", this);
        QObject::connect(checkbox.mCheckbox, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
        filterLayout->addWidget(checkbox.mCheckbox, row, col);
        ++col;
        if (col == 9)
        {
            col = 2;
            ++row;
        }
    }

    auto horLayout = new QHBoxLayout(this);
    horLayout->addLayout(filterLayout);
    horLayout->addStretch();
    mMainLayout->addLayout(horLayout);
}

void S2Plugin::ViewEntities::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewEntities::refreshEntities()
{
    mMainTreeView->clear();

    bool isUIDlookupSuccess = false;
    uint enteredUID = 0;
    if (!mFilterLineEdit->text().isEmpty())
    {
        enteredUID = mFilterLineEdit->text().toUInt(&isUIDlookupSuccess, 0);
    }

    auto AddEntity = [&](size_t entity_ptr)
    {
        auto entity = Entity{Script::Memory::ReadQword(entity_ptr)};
        QString entityName = QString::fromStdString(Configuration::get()->getEntityName(entity.entityTypeID()));

        if (!isUIDlookupSuccess && !mFilterLineEdit->text().isEmpty())
        {
            if (!entityName.contains(mFilterLineEdit->text(), Qt::CaseInsensitive))
                return;
        }

        MemoryField field;
        field.name = "entity_uid_" + std::to_string(entity.uid());
        field.type = MemoryFieldType::EntityPointer;
        field.isPointer = true;
        mMainTreeView->addMemoryField(field, "", entity_ptr, 0);
    };

    size_t totalEntities = 0;
    auto layer0 = Script::Memory::ReadQword(mLayer0Offset);
    auto layer0Count = Script::Memory::ReadDword(layer0 + 0x1C);
    auto layer1 = Script::Memory::ReadQword(mLayer1Offset);
    auto layer1Count = Script::Memory::ReadDword(layer1 + 0x1C);
    mCheckboxLayer0->setText(QString("Front layer (%1)").arg(layer0Count));
    mCheckboxLayer1->setText(QString("Back layer (%1)").arg(layer1Count));

    auto check_layer0 = mCheckboxLayer0->checkState() == Qt::Checked;
    auto check_layer1 = mCheckboxLayer1->checkState() == Qt::Checked;

    if (isUIDlookupSuccess)
    {
        // loop thru all entities to find the uid
        // TODO: change to proper struct when done
        auto ent_list = Script::Memory::ReadQword(layer0 + 0x8);
        auto uid_list = Script::Memory::ReadQword(layer0 + 0x10);
        bool found_uid = false;
        for (uint idx = 0; idx < layer0Count; ++idx)
        {
            auto uid = Script::Memory::ReadDword(uid_list + idx * sizeof(uint32_t));
            if (enteredUID == uid)
            {
                AddEntity(ent_list + idx * sizeof(size_t));
                found_uid = true;
                break;
            }
        }
        ent_list = Script::Memory::ReadQword(layer1 + 0x8);
        uid_list = Script::Memory::ReadQword(layer1 + 0x10);
        if (found_uid == false)
        {
            for (uint idx = 0; idx < layer1Count; ++idx)
            {
                auto uid = Script::Memory::ReadDword(uid_list + idx * sizeof(uint32_t));
                if (enteredUID == uid)
                {
                    AddEntity(ent_list + idx * sizeof(size_t));
                    break;
                }
            }
        }
    }

    StdMap<MASK, size_t> map0{layer0 + mLayerMapOffset};
    StdMap<MASK, size_t> map1{layer1 + mLayerMapOffset};

    for (auto& checkbox : mCheckbox)
    {
        int field_count = 0;

        if (check_layer0)
        {
            auto itr = map0.find(checkbox.mask);
            if (itr != map0.end())
            {
                // TODO: change to proper struct when done
                auto ent_list = itr.value_ptr();
                auto pointers = Script::Memory::ReadQword(ent_list);
                auto list_count = Script::Memory::ReadDword(ent_list + 20);
                field_count += list_count;
                // loop only if uid was not entered and the mask was choosen
                if (!isUIDlookupSuccess && totalEntities < 10000u && checkbox.mCheckbox->checkState() == Qt::Checked)
                {
                    for (size_t i = 0; i < list_count; ++i)
                    {
                        AddEntity(pointers + (i * sizeof(uintptr_t)));
                        ++totalEntities;
                    }
                }
            }
        }
        if (check_layer1)
        {
            auto itr = map1.find(checkbox.mask);
            if (itr != map1.end())
            {
                auto ent_list = itr.value_ptr();
                auto pointers = Script::Memory::ReadQword(ent_list);
                auto list_count = Script::Memory::ReadDword(ent_list + 20);
                field_count += list_count;
                if (!isUIDlookupSuccess && totalEntities < 10000u && checkbox.mCheckbox->checkState() == Qt::Checked)
                {
                    for (size_t i = 0; i < list_count; ++i)
                    {
                        AddEntity(pointers + (i * sizeof(size_t)));
                        ++totalEntities;
                    }
                }
            }
        }
        checkbox.mCheckbox->setText(QString(checkbox.name + " (%1)").arg(field_count));
    }
    setWindowTitle(QString("%1 Entities").arg(totalEntities));

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree();
}

QSize S2Plugin::ViewEntities::sizeHint() const
{
    return QSize(850, 550);
}

QSize S2Plugin::ViewEntities::minimumSizeHint() const
{
    return QSize(150, 150);
}
