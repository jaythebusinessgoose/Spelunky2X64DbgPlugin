#include "Views/ViewTextureDB.h"

#include "Configuration.h"
#include "Data/TextureDB.h"
#include "QtHelpers/DatabaseHelper.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include <QCheckBox>
#include <QCompleter>
#include <QHeaderView>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

S2Plugin::ViewTextureDB::ViewTextureDB(size_t index, QWidget* parent) : QWidget(parent)
{
    initializeUI();
    setWindowIcon(S2Plugin::getCavemanIcon());
    setWindowTitle(QString("Texture DB (%1 textures)").arg(Spelunky2::get()->get_TextureDB().count()));
    showID(index);
}

void S2Plugin::ViewTextureDB::initializeUI()
{
    auto mainLayout = new QVBoxLayout();
    mainLayout->setMargin(5);
    setLayout(mainLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    mTabLookup = new QWidget();  // ownership passed on via addTab
    mTabCompare = new QWidget(); // ovnership passed on via addTab
    mTabLookup->setLayout(new QVBoxLayout());
    mTabLookup->layout()->setMargin(10);
    mTabLookup->setObjectName("lookupwidget");
    mTabLookup->setStyleSheet("QWidget#lookupwidget {border: 1px solid #999;}");
    mTabCompare->setLayout(new QVBoxLayout());
    mTabCompare->layout()->setMargin(10);
    mTabCompare->setObjectName("comparewidget");
    mTabCompare->setStyleSheet("QWidget#comparewidget {border: 1px solid #999;}");

    mMainTabWidget->addTab(mTabLookup, "Lookup");
    mMainTabWidget->addTab(mTabCompare, "Compare");

    // LOOKUP
    {
        auto topLayout = new QHBoxLayout(this);

        mSearchLineEdit = new QLineEdit();
        mSearchLineEdit->setPlaceholderText("Search id");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewTextureDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        auto textureNameCompleter = new QCompleter(Spelunky2::get()->get_TextureDB().namesStringList(), this);
        textureNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        textureNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(textureNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewTextureDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(textureNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewTextureDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView = new TreeViewMemoryFields(this);
        mMainTreeView->setEnableChangeHighlighting(false);
        mMainTreeView->addMemoryFields(Configuration::get()->typeFields(MemoryFieldType::TextureDB), "TextureDB", 0);

        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewTextureDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewTextureDB::fieldExpanded);
        mTabLookup->layout()->addWidget(mMainTreeView);
        mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader();
    }

    // COMPARE
    {
        auto topLayout = new QHBoxLayout();
        mCompareFieldComboBox = new QComboBox(this);
        mCompareFieldComboBox->addItem(QString::fromStdString(""), QVariant::fromValue(QString::fromStdString("")));
        DB::populateComparisonCombobox(mCompareFieldComboBox, Configuration::get()->typeFields(MemoryFieldType::TextureDB));

        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewTextureDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewTextureDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(Spelunky2::get()->get_TextureDB().count(), 3, this);
        mCompareTableWidget->setAlternatingRowColors(true);
        mCompareTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mCompareTableWidget->setHorizontalHeaderLabels(QStringList() << "ID"
                                                                     << "Name"
                                                                     << "Value");
        mCompareTableWidget->verticalHeader()->setVisible(false);
        mCompareTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        mCompareTableWidget->verticalHeader()->setDefaultSectionSize(20);
        mCompareTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mCompareTableWidget->setColumnWidth(0, 40);
        mCompareTableWidget->setColumnWidth(1, 325);
        mCompareTableWidget->setColumnWidth(2, 150);
        auto HTMLDelegate = new StyledItemDelegateHTML(this);
        mCompareTableWidget->setItemDelegate(HTMLDelegate);
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewTextureDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(HTMLDelegate);
        QObject::connect(mCompareTreeWidget, &QTreeWidget::itemClicked, this, &ViewTextureDB::groupedComparisonItemClicked);

        mTabCompare->layout()->addWidget(mCompareTableWidget);
        mTabCompare->layout()->addWidget(mCompareTreeWidget);
    }

    mSearchLineEdit->setVisible(true);
    mSearchLineEdit->setFocus();
    mMainTreeView->setVisible(true);
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void S2Plugin::ViewTextureDB::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewTextureDB::sizeHint() const
{
    return QSize(750, 375);
}

QSize S2Plugin::ViewTextureDB::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewTextureDB::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    if (isNumeric && Spelunky2::get()->get_TextureDB().isValidID(enteredID))
    {
        showID(enteredID);
    }
    else
    {
        static const QRegularExpression r("^Texture ([0-9]+)");
        auto m = r.match(text);
        if (m.isValid())
        {
            auto textureID = m.captured(1).toUInt();
            showID(textureID);
        }
    }
}

void S2Plugin::ViewTextureDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewTextureDB::showID(uint32_t id)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    auto offset = Spelunky2::get()->get_TextureDB().addressOfID(id);
    mMainTreeView->updateTree(offset);
}

void S2Plugin::ViewTextureDB::label()
{
    auto model = mMainTreeView->model();
    std::string name;
    auto& textureDB = Spelunky2::get()->get_TextureDB();
    for (uint idx = 0; idx < model->rowCount(); ++idx)
    {
        if (model->data(model->index(idx, gsColField), Qt::DisplayRole).toString() == "id")
        {
            auto id = model->data(model->index(idx, gsColValue), gsRoleRawValue).toUInt();
            name = '[' + textureDB.nameForID(id) + ']';
            break;
        }
    }
    mMainTreeView->labelAll(name);
}

void S2Plugin::ViewTextureDB::fieldUpdated(int row, QStandardItem* parrent)
{
    if (parrent != nullptr) // special case: for flag field need to update it's parrent, not the flag field
    {
        auto model = qobject_cast<QStandardItemModel*>(mMainTreeView->model());
        auto parrentIndex = parrent->index();
        auto index = model->index(row, gsColField, parrentIndex);
        if (model->data(index, gsRoleType).value<MemoryFieldType>() == MemoryFieldType::Flag)
        {
            mMainTreeView->updateRow(parrentIndex.row(), std::nullopt, std::nullopt, parrent->parent(), true);
            return;
        }
    }
    mMainTreeView->updateRow(row, std::nullopt, std::nullopt, parrent, true);
}

void S2Plugin::ViewTextureDB::fieldExpanded(const QModelIndex& index)
{
    auto model = qobject_cast<QStandardItemModel*>(mMainTreeView->model());
    mMainTreeView->updateRow(index.row(), std::nullopt, std::nullopt, model->itemFromIndex(index.parent()), true);
}

void S2Plugin::ViewTextureDB::compareGroupByCheckBoxClicked(int state)
{
    mCompareTableWidget->setHidden(state == Qt::Checked);
    mCompareTreeWidget->setHidden(state == Qt::Unchecked);
}

void S2Plugin::ViewTextureDB::comparisonFieldChosen(const QString& fieldName)
{
    mCompareTableWidget->clearContents();
    mCompareTreeWidget->clear();

    auto comboIndex = mCompareFieldComboBox->currentIndex();
    if (comboIndex == 0)
    {
        return;
    }

    populateComparisonTableWidget();
    populateComparisonTreeWidget();
}

void S2Plugin::ViewTextureDB::populateComparisonTableWidget()
{
    mCompareTableWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& textureDB = Spelunky2::get()->get_TextureDB();

    size_t row = 0;
    for (auto& [textureID, textureData] : textureDB.textures())
    {
        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", textureID));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        auto name = QString("Texture %1 (%2)").arg(textureID).arg(QString::fromStdString(textureData.first));
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(name)));

        auto [caption, value] = DB::valueForField(comboboxData, textureData.second);
        auto item = new TableWidgetItemNumeric(caption);
        item->setData(Qt::UserRole, value);
        mCompareTableWidget->setItem(row, 2, item);

        row++;
    }
    mCompareTableWidget->setSortingEnabled(true);
    mCompareTableWidget->sortItems(0);
}

void S2Plugin::ViewTextureDB::populateComparisonTreeWidget()
{
    mCompareTreeWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& textureDB = Spelunky2::get()->get_TextureDB();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<texture id's>
    for (auto& [textureID, textureData] : textureDB.textures())
    {
        auto [caption, value] = DB::valueForField(comboboxData, textureData.second);
        auto captionStr = caption.toStdString();
        rootValues[captionStr] = value;

        if (groupedValues.count(captionStr) == 0)
        {
            groupedValues[captionStr] = {textureID};
        }
        else
        {
            groupedValues[captionStr].insert(textureID);
        }
    }

    for (const auto& [groupString, textureIds] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(nullptr, QString::fromStdString(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& textureId : textureIds)
        {
            auto textureName = QString("Texture %1 (%2)").arg(textureId).arg(QString::fromStdString(textureDB.nameForID(textureId)));
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(textureName);
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, textureId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void S2Plugin::ViewTextureDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        mSearchLineEdit->clear();
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toULongLong();
        showID(clickedID);
    }
}

void S2Plugin::ViewTextureDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        mSearchLineEdit->clear();
        showID(item->data(0, Qt::UserRole).toUInt());
    }
}
