#include "Views/ViewJournalPage.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewJournalPage::ViewJournalPage(uintptr_t offset, const std::string& pageType, QWidget* parent) : QWidget(parent), mOffset(offset), mPageType(pageType)
{
    initializeUI();
    setWindowIcon(S2Plugin::getCavemanIcon());
    setWindowTitle("JournalPage");

    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewJournalPage::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewJournalPage::refreshJournalPage);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewJournalPage::refreshJournalPage);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewJournalPage::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewJournalPage::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    mRefreshLayout->addWidget(new QLabel("Interpret as:", this));
    mInterpretAsComboBox = new QComboBox(this);
    // TODO get from json
    // also, guess page by the vtable
    mInterpretAsComboBox->addItem("JournalPage");
    mInterpretAsComboBox->addItem("JournalPageProgress");
    mInterpretAsComboBox->addItem("JournalPageJournalMenu");
    mInterpretAsComboBox->addItem("JournalPagePlaces");
    mInterpretAsComboBox->addItem("JournalPagePeople");
    mInterpretAsComboBox->addItem("JournalPageBestiary");
    mInterpretAsComboBox->addItem("JournalPageItems");
    mInterpretAsComboBox->addItem("JournalPageTraps");
    mInterpretAsComboBox->addItem("JournalPageStory");
    mInterpretAsComboBox->addItem("JournalPageFeats");
    mInterpretAsComboBox->addItem("JournalPageDeathCause");
    mInterpretAsComboBox->addItem("JournalPageDeathMenu");
    mInterpretAsComboBox->addItem("JournalPageRecap");
    mInterpretAsComboBox->addItem("JournalPagePlayerProfile");
    mInterpretAsComboBox->addItem("JournalPageLastGamePlayed");

    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewJournalPage::interpretAsChanged);
    mRefreshLayout->addWidget(mInterpretAsComboBox);
    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewJournalPage::label);
    mRefreshLayout->addWidget(labelButton);

    mMainTreeView = new TreeViewMemoryFields(this);
    mMainTreeView->addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(mPageType), mPageType, mOffset);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->updateTableHeader();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewJournalPage::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewJournalPage::refreshJournalPage()
{
    mMainTreeView->updateTree();
}

void S2Plugin::ViewJournalPage::toggleAutoRefresh(int newState)
{
    if (newState == Qt::Unchecked)
    {
        mAutoRefreshTimer->stop();
        mRefreshButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::ViewJournalPage::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

QSize S2Plugin::ViewJournalPage::sizeHint() const
{
    return QSize(750, 750);
}

QSize S2Plugin::ViewJournalPage::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewJournalPage::label()
{
    mMainTreeView->labelAll();
}

void S2Plugin::ViewJournalPage::interpretAsChanged(const QString& text)
{
    if (!text.isEmpty())
    {
        mPageType = text.toStdString();
        mMainTreeView->clear();
        mMainTreeView->addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(mPageType), mPageType, mOffset);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->updateTableHeader();
        mMainTreeView->updateTree(0, 0, true);
        // mInterpretAsComboBox->setCurrentText("");
    }
}
